/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** reliable_channel_test
*/

#include "common/protocol/ReliableChannel.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

using namespace rtype::protocol;
using namespace std::chrono_literals;

namespace {

using TimePoint = ReliableChannel::TimePoint;

Packet eventPacket(std::uint32_t networkId)
{
    const GameEvent event{
        .kind = GameEventKind::EntityDestroyed,
        .networkId = networkId,
    };
    return {.payload = event};
}

Packet inputPacket() { return {.payload = Input{}}; }

Packet sent(ReliableChannel &channel, Packet packet, TimePoint now = {})
{
    EXPECT_TRUE(channel.prepare(packet, now));
    return packet;
}

std::uint32_t eventId(const Packet &packet)
{
    return std::get<GameEvent>(packet.payload).networkId;
}

// Loses 30% of the packets, duplicates 10% and delivers them in random order.
class LossyLink {
  public:
    explicit LossyLink(std::uint32_t seed) : m_random(seed) {}

    void send(const Packet &packet)
    {
        if (m_random() % 100 < 30) {
            return;
        }
        m_inFlight.push_back(packet);
        if (m_random() % 100 < 10) {
            m_inFlight.push_back(packet);
        }
    }

    std::vector<Packet> take()
    {
        std::ranges::shuffle(m_inFlight, m_random);
        return std::exchange(m_inFlight, {});
    }

  private:
    std::mt19937 m_random;
    std::vector<Packet> m_inFlight;
};

// The server sends numbered events to the client over two lossy links,
// while the client sends one input per step, carrying its ack.
class Simulation {
  public:
    static constexpr std::uint32_t EventCount = 500;

    void step()
    {
        m_now += 10ms;
        if (m_nextEvent < EventCount && m_server.pendingCount() < 8) {
            m_toClient.send(sent(m_server, eventPacket(m_nextEvent++), m_now));
        }
        for (const Packet &packet : m_server.resendsDue(m_now)) {
            m_toClient.send(packet);
        }
        for (const Packet &packet : m_toClient.take()) {
            if (m_client.receive(packet.header)) {
                m_delivered.push_back(eventId(packet));
            }
        }
        m_toServer.send(sent(m_client, inputPacket(), m_now));
        for (const Packet &packet : m_toServer.take()) {
            EXPECT_TRUE(m_server.receive(packet.header));
        }
    }

    [[nodiscard]] bool done() const
    {
        return m_delivered.size() == EventCount && m_server.pendingCount() == 0;
    }

    [[nodiscard]] const std::vector<std::uint32_t> &delivered() const
    {
        return m_delivered;
    }

  private:
    ReliableChannel m_server;
    ReliableChannel m_client;
    LossyLink m_toClient{1};
    LossyLink m_toServer{2};
    std::vector<std::uint32_t> m_delivered;
    std::uint32_t m_nextEvent = 0;
    TimePoint m_now;
};

} // namespace

TEST(ReliableChannel, NumbersReliableAndUnreliablePacketsSeparately)
{
    ReliableChannel channel;
    EXPECT_EQ(sent(channel, inputPacket()).header.sequence, 0);
    EXPECT_EQ(sent(channel, eventPacket(1)).header.sequence, 1);
    EXPECT_EQ(sent(channel, inputPacket()).header.sequence, 1);
    EXPECT_EQ(sent(channel, eventPacket(2)).header.sequence, 2);
    EXPECT_EQ(channel.pendingCount(), 2);
}

TEST(ReliableChannel, DeliversReliablePacketsInOrderAndOnce)
{
    ReliableChannel sender;
    ReliableChannel receiver;
    const Packet first = sent(sender, eventPacket(1));
    const Packet second = sent(sender, eventPacket(2));
    EXPECT_FALSE(receiver.receive(second.header)); // first was lost
    EXPECT_TRUE(receiver.receive(first.header));
    EXPECT_FALSE(receiver.receive(first.header)); // duplicate
    EXPECT_TRUE(receiver.receive(second.header));
    EXPECT_EQ(receiver.lastInOrder(), 2);
}

TEST(ReliableChannel, AlwaysDeliversUnreliablePackets)
{
    ReliableChannel sender;
    ReliableChannel receiver;
    const Packet input = sent(sender, inputPacket());
    EXPECT_TRUE(receiver.receive(input.header));
    EXPECT_TRUE(receiver.receive(input.header));
    EXPECT_FALSE(receiver.needsAck());
}

TEST(ReliableChannel, EveryPacketCarriesTheAck)
{
    ReliableChannel sender;
    ReliableChannel receiver;
    ASSERT_TRUE(receiver.receive(sent(sender, eventPacket(1)).header));
    EXPECT_TRUE(receiver.needsAck());
    EXPECT_EQ(sent(receiver, inputPacket()).header.ack, 1);
    EXPECT_FALSE(receiver.needsAck());
}

TEST(ReliableChannel, DuplicatesAreAcknowledgedAgain)
{
    ReliableChannel sender;
    ReliableChannel receiver;
    const Packet event = sent(sender, eventPacket(1));
    ASSERT_TRUE(receiver.receive(event.header));
    (void)sent(receiver, inputPacket()); // this ack is lost
    EXPECT_FALSE(receiver.receive(event.header));
    EXPECT_TRUE(receiver.needsAck());
}

TEST(ReliableChannel, CumulativeAckForgetsAcknowledgedPackets)
{
    ReliableChannel sender;
    for (std::uint32_t id = 1; id <= 3; ++id) {
        (void)sent(sender, eventPacket(id));
    }
    PacketHeader ack{.type = PacketType::Ack, .ack = 2};
    EXPECT_TRUE(sender.receive(ack));
    EXPECT_EQ(sender.pendingCount(), 1);
    ack.ack = 1; // older ack, arrived late
    EXPECT_TRUE(sender.receive(ack));
    EXPECT_EQ(sender.pendingCount(), 1);
    ack.ack = 3;
    EXPECT_TRUE(sender.receive(ack));
    EXPECT_EQ(sender.pendingCount(), 0);
}

TEST(ReliableChannel, IgnoresAckOfPacketsNeverSent)
{
    ReliableChannel sender;
    (void)sent(sender, eventPacket(1));
    EXPECT_TRUE(sender.receive({.type = PacketType::Ack, .ack = 500}));
    EXPECT_EQ(sender.pendingCount(), 1);
}

TEST(ReliableChannel, ResendsEvery100Milliseconds)
{
    ReliableChannel sender;
    const TimePoint start{};
    const Packet event = sent(sender, eventPacket(1), start);
    EXPECT_TRUE(sender.resendsDue(start + 99ms).empty());
    ASSERT_TRUE(sender.receive({.type = PacketType::Welcome, .sequence = 1}));
    const auto due = sender.resendsDue(start + 100ms);
    ASSERT_EQ(due.size(), 1);
    EXPECT_EQ(due[0].header.sequence, event.header.sequence);
    EXPECT_EQ(due[0].header.ack, 1); // up to date
    EXPECT_FALSE(sender.needsAck());
    EXPECT_TRUE(sender.resendsDue(start + 150ms).empty());
    EXPECT_EQ(sender.resendsDue(start + 200ms).size(), 1);
}

TEST(ReliableChannel, RefusesTooManyPendingPackets)
{
    ReliableChannel sender;
    for (std::size_t i = 0; i < MaxPendingReliable; ++i) {
        Packet packet = eventPacket(1);
        ASSERT_TRUE(sender.prepare(packet, {}));
    }
    Packet event = eventPacket(1);
    EXPECT_FALSE(sender.prepare(event, {}));
    Packet input = inputPacket();
    EXPECT_TRUE(sender.prepare(input, {}));
}

TEST(ReliableChannel, SurvivesSequenceWrapAround)
{
    ReliableChannel sender;
    ReliableChannel receiver;
    for (std::uint32_t id = 0; id < 70000; ++id) {
        ASSERT_TRUE(receiver.receive(sent(sender, eventPacket(id)).header));
        ASSERT_TRUE(sender.receive(sent(receiver, inputPacket()).header));
        ASSERT_EQ(sender.pendingCount(), 0);
    }
}

TEST(ReliableChannel, DeliversEverythingOverALossyLink)
{
    Simulation simulation;
    for (int step = 0; step < 100000 && !simulation.done(); ++step) {
        simulation.step();
    }
    const auto &delivered = simulation.delivered();
    ASSERT_EQ(delivered.size(), Simulation::EventCount);
    for (std::uint32_t id = 0; id < Simulation::EventCount; ++id) {
        EXPECT_EQ(delivered[id], id);
    }
}
