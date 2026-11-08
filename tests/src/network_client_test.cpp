/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** network_client_test
*/

#include "FakeNetwork.hpp"
#include "FakeRooms.hpp"
#include "network/NetworkClient.hpp"
#include "network/NetworkMessages.hpp"
#include "network/NetworkServer.hpp"

#include <gtest/gtest.h>

#include <algorithm>

#include <array>
#include <chrono>
#include <cstdint>
#include <vector>

using namespace rtype::protocol;
using namespace std::chrono_literals;
using client::NetworkClient;
using engine::net::Endpoint;
using tests::FakeNetwork;
using tests::FakePeer;

namespace {

const Endpoint ServerEndpoint{.address = 0x0A000001, .port = 4242};
const Endpoint ClientEndpoint{.address = 0x0A000002, .port = 5000};

// Sends an Accept to the client from a raw socket.
void sendAccept(engine::net::IUdpSocket &server)
{
    ReliableChannel channel;
    Packet accept{.payload = Accept{}};
    EXPECT_TRUE(channel.prepare(accept, std::chrono::steady_clock::now()));
    ByteWriter writer;
    EXPECT_TRUE(encodePacket(accept, writer));
    EXPECT_FALSE(
        server.sendTo(ClientEndpoint, {writer.data().data(), writer.size()}));
}

// Counts the JoinRoom datagrams waiting on a raw socket, resends included.
int countJoinRooms(engine::net::IUdpSocket &server)
{
    int requests = 0;
    std::array<std::uint8_t, MaxPacketSize> buffer{};
    while (const auto datagram = server.receive(buffer, 0ms)) {
        const auto result =
            decodePacket({buffer.data(), datagram->size}, Side::Client);
        const auto *packet = std::get_if<Packet>(&result);
        if (packet != nullptr &&
            std::holds_alternative<JoinRoom>(packet->payload)) {
            ++requests;
        }
    }
    return requests;
}

// Waits until a game received at least this number of messages.
std::vector<server::InboundMessage> waitInbound(server::Inbound &inbound,
                                                std::size_t count)
{
    std::vector<server::InboundMessage> received;
    EXPECT_TRUE(tests::eventually([&] {
        for (const auto &message : inbound.drain()) {
            received.push_back(message);
        }
        return received.size() >= count;
    }));
    return received;
}

// Asks for a room and waits until the client is in one.
bool enterAnyRoom(NetworkClient &client)
{
    client.joinRoom(AnyRoom, std::chrono::steady_clock::now());
    return tests::eventually([&] {
        const auto payloads = client.update(std::chrono::steady_clock::now());
        return std::ranges::any_of(payloads, [](const Payload &payload) {
            return std::holds_alternative<RoomJoined>(payload);
        });
    });
}

// A NetworkClient facing a hand-driven server.
class NetworkClientTest : public ::testing::Test {
  public:
    FakeNetwork m_network;
    FakePeer m_server{m_network, ServerEndpoint, ClientEndpoint, Side::Client};
    NetworkClient m_client{m_network.open(ClientEndpoint), ServerEndpoint};
    NetworkClient::TimePoint m_now = std::chrono::steady_clock::now();

    std::vector<Payload> update()
    {
        m_now += 1ms;
        return m_client.update(m_now);
    }

    void connect()
    {
        static_cast<void>(update());
        ASSERT_TRUE(m_server.waitFor<Connect>().has_value());
        m_server.send(Accept{});
        static_cast<void>(update());
        ASSERT_EQ(m_client.state(), NetworkClient::State::Connected);
    }
};

} // namespace

TEST_F(NetworkClientTest, HandshakeConnectsTheClient)
{
    EXPECT_EQ(m_client.state(), NetworkClient::State::Connecting);

    connect();
}

TEST_F(NetworkClientTest, HandsGamePayloadsToTheGame)
{
    connect();
    const Welcome welcome{.slot = 1, .shipId = 42, .tick = 600};

    m_server.send(welcome);

    const auto received = update();
    ASSERT_EQ(received.size(), 1U);
    EXPECT_EQ(std::get<Welcome>(received.front()), welcome);
}

TEST_F(NetworkClientTest, SendsInputsOnceConnected)
{
    connect();

    m_client.sendInput(0b11, m_now);

    EXPECT_EQ(m_server.waitFor<Input>(),
              (Input{.sequence = 1, .actions = 0b11}));
}

TEST_F(NetworkClientTest, LimitsTheInputRateWhenTheActionsDoNotChange)
{
    connect();
    constexpr int Frames = 240;

    std::uint32_t sequence = 0;
    for (int frame = 0; frame < Frames; ++frame) {
        m_now += 4ms;
        sequence = m_client.sendInput(0b1, m_now);
    }

    EXPECT_GE(sequence, 30U);
    EXPECT_LE(sequence, 35U);
}

TEST_F(NetworkClientTest, SendsChangedActionsAtOnce)
{
    connect();
    EXPECT_EQ(m_client.sendInput(0b1, m_now), 1U);
    ASSERT_TRUE(m_server.waitFor<Input>().has_value());

    m_now += 1ms;
    EXPECT_EQ(m_client.sendInput(0b1, m_now), 2U);
    m_now += 1ms;
    EXPECT_EQ(m_client.sendInput(0b10, m_now), 2U);

    EXPECT_EQ(m_server.waitFor<Input>(),
              (Input{.sequence = 2, .actions = 0b10}));
}

TEST_F(NetworkClientTest, IgnoresMalformedAndForeignDatagrams)
{
    static_cast<void>(update());
    std::array<std::uint8_t, 64> garbage{};
    garbage.fill(0xAB);
    FakePeer stranger(m_network, {.address = 0x0A000003, .port = 4242},
                      ClientEndpoint, Side::Client);

    m_server.sendRaw(garbage);
    m_server.sendRaw({});
    stranger.send(Accept{});

    EXPECT_TRUE(update().empty());
    EXPECT_EQ(m_client.state(), NetworkClient::State::Connecting);
    m_server.send(Accept{});
    static_cast<void>(update());
    EXPECT_EQ(m_client.state(), NetworkClient::State::Connected);
}

TEST(NetworkClient, ResendsARoomRequestUntilAcknowledged)
{
    FakeNetwork network;
    const auto server = network.open(ServerEndpoint);
    NetworkClient client(network.open(ClientEndpoint), ServerEndpoint);
    auto now = std::chrono::steady_clock::now();
    static_cast<void>(client.update(now));
    sendAccept(*server);
    static_cast<void>(client.update(now));
    ASSERT_EQ(client.state(), NetworkClient::State::Connected);

    client.joinRoom(5, now);
    now += 200ms;
    static_cast<void>(client.update(now));

    EXPECT_EQ(countJoinRooms(*server), 2);
}

TEST(NetworkClientServer, ConnectAndPlayOverTheSocketInterface)
{
    FakeNetwork network;
    tests::FakeRooms fakeRooms;
    server::Inbound &inbound = fakeRooms.inbound;
    server::NetworkServer server(network.open(ServerEndpoint), fakeRooms.rooms);
    server.start();
    NetworkClient client(network.open(ClientEndpoint), ServerEndpoint);

    const bool connected = tests::eventually([&] {
        static_cast<void>(client.update(std::chrono::steady_clock::now()));
        return client.state() == NetworkClient::State::Connected;
    });
    ASSERT_TRUE(connected);
    ASSERT_TRUE(enterAnyRoom(client));
    client.sendInput(0b1, std::chrono::steady_clock::now());

    const auto received = waitInbound(inbound, 2);
    ASSERT_GE(received.size(), 2U);
    EXPECT_EQ(received[0].kind, server::InboundKind::Connected);
    EXPECT_EQ(received[1].kind, server::InboundKind::Input);
    EXPECT_EQ(received[1].actions, 0b1);
}
