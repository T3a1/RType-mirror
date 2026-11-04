/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** network_server_test
*/

#include "FakeNetwork.hpp"
#include "FakeRooms.hpp"
#include "network/NetworkMessages.hpp"
#include "network/NetworkServer.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

using namespace rtype::protocol;
using engine::net::Endpoint;
using tests::FakeNetwork;
using tests::FakePeer;

namespace {

const Endpoint ServerEndpoint{.address = 0x0A000001, .port = 4242};

Endpoint clientEndpoint(std::uint16_t port)
{
    return {.address = 0x0A000002, .port = port};
}

// A NetworkServer running on its own thread over a FakeNetwork.
class NetworkServerTest : public ::testing::Test {
  public:
    FakeNetwork m_network;
    tests::FakeRooms m_fakeRooms;
    server::Inbound &m_inbound = m_fakeRooms.inbound;
    server::Outbound &m_outbound = m_fakeRooms.outbound;
    server::NetworkServer m_server{m_network.open(ServerEndpoint),
                                   m_fakeRooms.rooms};
    std::vector<server::InboundMessage> m_received;

    void SetUp() override { m_server.start(); }

    FakePeer peer(std::uint16_t port)
    {
        return {m_network, clientEndpoint(port), ServerEndpoint, Side::Server};
    }

    // Waits for an inbound message of a kind, keeping the others. Fails the
    // test and returns a default message if none comes.
    server::InboundMessage waitInbound(server::InboundKind kind)
    {
        std::optional<server::InboundMessage> found;
        const bool arrived = tests::eventually([&] {
            for (const auto &message : m_inbound.drain()) {
                m_received.push_back(message);
            }
            for (const auto &message : m_received) {
                if (message.kind == kind) {
                    found = message;
                    return true;
                }
            }
            return false;
        });
        EXPECT_TRUE(arrived);
        return found.value_or(server::InboundMessage{});
    }

    // Puts a peer in a room and returns its ClientId.
    std::uint32_t connect(FakePeer &client)
    {
        client.send(Connect{});
        EXPECT_TRUE(client.waitFor<Accept>().has_value());
        client.send(JoinRoom{});
        EXPECT_TRUE(client.waitFor<RoomJoined>().has_value());
        const auto connected = waitInbound(server::InboundKind::Connected);
        m_received.clear();
        return connected.clientId;
    }
};

} // namespace

TEST_F(NetworkServerTest, HandshakeAcceptsTheClientAndTellsTheGame)
{
    FakePeer client = peer(5000);
    client.send(Connect{});
    EXPECT_TRUE(client.waitFor<Accept>().has_value());
    client.send(JoinRoom{});

    EXPECT_TRUE(client.waitFor<RoomJoined>().has_value());
    const auto connected = waitInbound(server::InboundKind::Connected);
    EXPECT_EQ(connected.kind, server::InboundKind::Connected);
    EXPECT_EQ(connected.clientId, 1U);
}

TEST_F(NetworkServerTest, ForwardsInputsToTheGame)
{
    FakePeer client = peer(5000);
    const std::uint32_t id = connect(client);

    client.send(Input{.sequence = 7, .actions = 0b101});

    const auto input = waitInbound(server::InboundKind::Input);
    EXPECT_EQ(input.clientId, id);
    EXPECT_EQ(input.sequence, 7U);
    EXPECT_EQ(input.actions, 0b101);
}

TEST_F(NetworkServerTest, SendsOutboundMessagesToTheirRecipients)
{
    FakePeer first = peer(5000);
    FakePeer second = peer(5001);
    const std::uint32_t firstId = connect(first);
    connect(second);
    const GameEvent event{.kind = GameEventKind::PlayerHit, .slot = 2};

    EXPECT_TRUE(m_outbound.push({
        .to = server::Recipient::One,
        .cliendId = firstId,
        .payload = event,
    }));

    EXPECT_EQ(first.waitFor<GameEvent>(), event);
    EXPECT_FALSE(
        second.waitFor<GameEvent>(std::chrono::milliseconds(100)).has_value());
}

TEST_F(NetworkServerTest, SurvivesMalformedDatagrams)
{
    FakePeer client = peer(5000);
    const std::array<std::uint8_t, 3> truncated{0x52, 0x54, 0x01};
    std::array<std::uint8_t, MaxPacketSize> garbage{};
    garbage.fill(0xFF);
    const std::vector<std::uint8_t> oversized(MaxPacketSize * 2, 0x42);

    client.sendRaw({});
    client.sendRaw(truncated);
    client.sendRaw(garbage);
    client.sendRaw(oversized);

    EXPECT_FALSE(
        client.waitFor<Accept>(std::chrono::milliseconds(100)).has_value());
    EXPECT_TRUE(m_inbound.drain().empty());
    client.send(Connect{});
    EXPECT_TRUE(client.waitFor<Accept>().has_value());
}

TEST_F(NetworkServerTest, RefusesARoomPastMaxPlayers)
{
    std::vector<FakePeer> clients;
    for (std::uint16_t i = 0; i < MaxPlayers; ++i) {
        clients.push_back(peer(5000 + i));
        connect(clients.back());
    }
    FakePeer late = peer(6000);

    late.send(Connect{});
    EXPECT_TRUE(late.waitFor<Accept>().has_value());
    late.send(JoinRoom{});

    EXPECT_TRUE(late.waitFor<RoomRefused>().has_value());
}

TEST_F(NetworkServerTest, ReportsAClientThatLeaves)
{
    FakePeer client = peer(5000);
    const std::uint32_t id = connect(client);

    client.send(Disconnect{.reason = DisconnectReason::Left});

    const auto left = waitInbound(server::InboundKind::Disconnected);
    EXPECT_EQ(left.kind, server::InboundKind::Disconnected);
    EXPECT_EQ(left.clientId, id);
    EXPECT_EQ(left.reason, DisconnectReason::Left);
}

TEST_F(NetworkServerTest, StopTellsEveryClientTheServerCloses)
{
    FakePeer client = peer(5000);
    connect(client);

    m_server.stop();

    EXPECT_EQ(client.waitFor<Disconnect>(),
              Disconnect{.reason = DisconnectReason::ServerClosing});
}
