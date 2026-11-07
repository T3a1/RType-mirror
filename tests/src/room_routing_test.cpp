/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** room_routing_test
*/

#include "FakeNetwork.hpp"
#include "engine/IInstance.hpp"
#include "network/NetworkMessages.hpp"
#include "network/NetworkServer.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <stdexcept>

using namespace rtype::protocol;
using engine::net::Endpoint;
using tests::FakeNetwork;
using tests::FakePeer;

namespace {

const Endpoint ServerEndpoint{.address = 0x0A000001, .port = 4242};

// A game that sends a PlayerHit event to its whole room for each input, so
// that a test sees who receives what. It fails on the action `FailAction`.
constexpr std::uint8_t FailAction = 0x10;
// The slot of the event it sends when asked to start.
constexpr std::uint8_t StartedSlot = 4;

class EchoGame final : public engine::IInstance {
  public:
    explicit EchoGame(server::Rooms::Link &link) : m_link(link) {}

    void tick(engine::Duration /*dt*/) override
    {
        for (const auto &message : m_link.inbound.drain()) {
            if (message.kind == server::InboundKind::Start) {
                send(StartedSlot);
            }
            if (message.kind == server::InboundKind::Chat) {
                m_link.outbound.push(server::OutboundMessage{
                    .to = server::Recipient::All,
                    .payload = ChatMessage{.slot = 1, .text = message.text},
                });
            }
            if (message.kind != server::InboundKind::Input) {
                continue;
            }
            if (message.actions == FailAction) {
                throw std::runtime_error("test failure");
            }
            send(message.actions);
        }
    }

  private:
    void send(std::uint8_t slot)
    {
        m_link.outbound.push(server::OutboundMessage{
            .to = server::Recipient::All,
            .payload =
                GameEvent{
                    .kind = GameEventKind::PlayerHit,
                    .slot = slot,
                },
        });
    }

    server::Rooms::Link &m_link;
};

class RoomRoutingTest : public ::testing::Test {
  public:
    FakeNetwork m_network;
    std::atomic<int> m_created{0};
    server::Rooms m_rooms{
        [this](server::Rooms::Link &link) {
            ++m_created;
            return std::make_unique<EchoGame>(link);
        },
        {.maxInstances = 2, .maxMembers = 2},
        std::chrono::milliseconds(1),
    };
    server::NetworkServer m_server{m_network.open(ServerEndpoint), m_rooms};

    FakePeer peer(std::uint16_t port)
    {
        return {m_network,
                {.address = 0x0A000002, .port = port},
                ServerEndpoint,
                Side::Server};
    }

    // Connects a peer and puts it in a room; returns the room it got.
    static std::uint32_t join(FakePeer &client, std::uint32_t room)
    {
        client.send(Connect{});
        EXPECT_TRUE(client.waitFor<Accept>().has_value());
        return enter(client, room);
    }

    // Puts a peer of the lobby in a room; returns the room it got.
    static std::uint32_t enter(FakePeer &client, std::uint32_t room)
    {
        client.send(JoinRoom{.room = room});
        const auto joined = client.waitFor<RoomJoined>();
        EXPECT_TRUE(joined.has_value());
        return joined.value_or(RoomJoined{.room = AnyRoom}).room;
    }

    // The slot of the next event a peer receives, which must come.
    static std::uint8_t nextEventSlot(FakePeer &client)
    {
        const auto event = client.waitFor<GameEvent>();
        EXPECT_TRUE(event.has_value());
        return event.value_or(GameEvent{}).slot;
    }

  protected:
    void SetUp() override { m_server.start(); }
};

} // namespace

TEST_F(RoomRoutingTest, ANewClientIsInTheLobby)
{
    FakePeer client = peer(1000);
    client.send(Connect{});

    EXPECT_TRUE(client.waitFor<Accept>().has_value());
    EXPECT_EQ(client.waitFor<RoomList>(), RoomList{});
    EXPECT_EQ(m_created, 0);
}

TEST_F(RoomRoutingTest, TheLobbyListsTheRooms)
{
    FakePeer player = peer(1000);
    FakePeer watcher = peer(1001);
    join(player, 5);
    watcher.send(Connect{});

    const RoomList expected{.rooms = {{.room = 5, .players = 1}}};
    EXPECT_TRUE(tests::eventually(
        [&] { return watcher.waitFor<RoomList>() == expected; }));
}

TEST_F(RoomRoutingTest, RoomJoinedNamesTheRequestedRoom)
{
    FakePeer client = peer(1000);

    EXPECT_EQ(join(client, 5), 5U);
}

TEST_F(RoomRoutingTest, NewRoomAlwaysOpensAnother)
{
    FakePeer first = peer(1000);
    FakePeer second = peer(1001);

    const std::uint32_t room = join(first, NewRoom);

    EXPECT_NE(room, AnyRoom);
    EXPECT_NE(join(second, NewRoom), room);
}

TEST_F(RoomRoutingTest, AnyRoomFillsARoomThenOpensAnother)
{
    FakePeer first = peer(1000);
    FakePeer second = peer(1001);
    FakePeer third = peer(1002);

    const std::uint32_t room = join(first, AnyRoom);

    EXPECT_NE(room, AnyRoom);
    EXPECT_EQ(join(second, AnyRoom), room);
    EXPECT_NE(join(third, AnyRoom), room);
}

TEST_F(RoomRoutingTest, ARoomOnlyTalksToItsPlayers)
{
    FakePeer inFirst = peer(1000);
    FakePeer alsoInFirst = peer(1001);
    FakePeer inSecond = peer(1002);
    join(inFirst, 1);
    join(alsoInFirst, 1);
    join(inSecond, 2);

    inFirst.send(Input{.sequence = 1, .actions = 3});

    EXPECT_EQ(nextEventSlot(inFirst), 3);
    EXPECT_EQ(nextEventSlot(alsoInFirst), 3);
    EXPECT_FALSE(inSecond.waitFor<GameEvent>(std::chrono::milliseconds(100))
                     .has_value());
}

TEST_F(RoomRoutingTest, RefusesAFullRoomAndTooManyRooms)
{
    FakePeer first = peer(1000);
    FakePeer second = peer(1001);
    FakePeer third = peer(1002);
    FakePeer fourth = peer(1003);
    join(first, 1);
    join(second, 1);
    join(third, 2);
    fourth.send(Connect{});
    EXPECT_TRUE(fourth.waitFor<Accept>().has_value());

    fourth.send(JoinRoom{.room = 1});
    EXPECT_TRUE(fourth.waitFor<RoomRefused>().has_value());
    fourth.send(JoinRoom{.room = 3});
    EXPECT_TRUE(fourth.waitFor<RoomRefused>().has_value());
}

TEST_F(RoomRoutingTest, LeavingGoesBackToTheLobbyAndDestroysAnEmptyRoom)
{
    FakePeer client = peer(1000);
    join(client, 1);

    client.send(LeaveRoom{});

    EXPECT_EQ(client.waitFor<RoomList>(), RoomList{});
    EXPECT_EQ(enter(client, 1), 1U);
    EXPECT_EQ(m_created, 2);
}

TEST_F(RoomRoutingTest, InputsAreIgnoredInTheLobby)
{
    FakePeer client = peer(1000);
    client.send(Connect{});
    EXPECT_TRUE(client.waitFor<Accept>().has_value());

    client.send(Input{.sequence = 1, .actions = 3});

    EXPECT_FALSE(
        client.waitFor<GameEvent>(std::chrono::milliseconds(100)).has_value());
}

TEST_F(RoomRoutingTest, StartReachesTheGameOfTheRoom)
{
    FakePeer client = peer(1000);
    join(client, 1);

    client.send(StartGame{});

    EXPECT_EQ(nextEventSlot(client), StartedSlot);
}

TEST_F(RoomRoutingTest, ChatStaysInTheRoom)
{
    FakePeer author = peer(1000);
    FakePeer sameRoom = peer(1001);
    FakePeer otherRoom = peer(1002);
    join(author, 1);
    join(sameRoom, 1);
    join(otherRoom, 2);

    author.send(ChatSend{.text = "hello"});

    const ChatMessage expected{.slot = 1, .text = "hello"};
    EXPECT_EQ(author.waitFor<ChatMessage>(), expected);
    EXPECT_EQ(sameRoom.waitFor<ChatMessage>(), expected);
    EXPECT_FALSE(otherRoom.waitFor<ChatMessage>(std::chrono::milliseconds(100))
                     .has_value());
}

TEST_F(RoomRoutingTest, ChatIsIgnoredInTheLobby)
{
    FakePeer client = peer(1000);
    client.send(Connect{});
    EXPECT_TRUE(client.waitFor<Accept>().has_value());

    client.send(ChatSend{.text = "hello"});

    EXPECT_FALSE(client.waitFor<ChatMessage>(std::chrono::milliseconds(100))
                     .has_value());
}

TEST_F(RoomRoutingTest, AFloodOfChatIsLimited)
{
    FakePeer client = peer(1000);
    join(client, 1);
    const int burst = static_cast<int>(server::ChatBurst);

    for (int i = 0; i < burst * 4; ++i) {
        client.send(ChatSend{.text = "spam"});
    }

    int received = 0;
    while (client.waitFor<ChatMessage>(std::chrono::milliseconds(100))) {
        ++received;
    }
    EXPECT_GE(received, burst);
    EXPECT_LE(received, burst + 2);
}

TEST_F(RoomRoutingTest, AFailingRoomOnlyDisconnectsItsPlayers)
{
    FakePeer failing = peer(1000);
    FakePeer healthy = peer(1001);
    join(failing, 1);
    join(healthy, 2);

    failing.send(Input{.sequence = 1, .actions = FailAction});

    const auto closed = failing.waitFor<Disconnect>();
    EXPECT_TRUE(closed.has_value());
    EXPECT_EQ(closed.value_or(Disconnect{}).reason,
              DisconnectReason::ServerClosing);
    healthy.send(Input{.sequence = 1, .actions = 2});
    EXPECT_EQ(nextEventSlot(healthy), 2);
}
