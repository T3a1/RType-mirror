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

class EchoGame final : public engine::IInstance {
  public:
    explicit EchoGame(server::Rooms::Link &link) : m_link(link) {}

    void tick(engine::Duration /*dt*/) override
    {
        for (const auto &message : m_link.inbound.drain()) {
            if (message.kind != server::InboundKind::Input) {
                continue;
            }
            if (message.actions == FailAction) {
                throw std::runtime_error("test failure");
            }
            m_link.outbound.push(server::OutboundMessage{
                .to = server::Recipient::All,
                .payload =
                    GameEvent{
                        .kind = GameEventKind::PlayerHit,
                        .slot = message.actions,
                    },
            });
        }
    }

  private:
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

    // Connects a peer to a room and returns the room it got.
    static std::uint32_t join(FakePeer &client, std::uint32_t room)
    {
        client.send(Connect{.room = room});
        const auto accept = client.waitFor<Accept>();
        EXPECT_TRUE(accept.has_value());
        return accept ? accept->room : AnyRoom;
    }

  protected:
    void SetUp() override { m_server.start(); }
};

} // namespace

TEST_F(RoomRoutingTest, AcceptNamesTheRequestedRoom)
{
    FakePeer client = peer(1000);

    EXPECT_EQ(join(client, 5), 5U);
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

    const auto received = inFirst.waitFor<GameEvent>();
    const auto alsoReceived = alsoInFirst.waitFor<GameEvent>();
    ASSERT_TRUE(received && alsoReceived);
    EXPECT_EQ(received->slot, 3);
    EXPECT_EQ(alsoReceived->slot, 3);
    EXPECT_FALSE(inSecond.waitFor<GameEvent>(std::chrono::milliseconds(100))
                     .has_value());
}

TEST_F(RoomRoutingTest, RejectsAFullRoomAndTooManyRooms)
{
    FakePeer first = peer(1000);
    FakePeer second = peer(1001);
    FakePeer third = peer(1002);
    FakePeer fourth = peer(1003);
    join(first, 1);
    join(second, 1);
    join(third, 2);

    fourth.send(Connect{.room = 1});
    const auto full = fourth.waitFor<Reject>();
    fourth.send(Connect{.room = 3});
    const auto tooMany = fourth.waitFor<Reject>();

    ASSERT_TRUE(full && tooMany);
    EXPECT_EQ(full->reason, RejectReason::ServerFull);
    EXPECT_EQ(tooMany->reason, RejectReason::ServerFull);
}

TEST_F(RoomRoutingTest, ARoomIsDestroyedAfterItsLastPlayer)
{
    FakePeer client = peer(1000);
    join(client, 1);

    client.send(Disconnect{.reason = DisconnectReason::Left});
    FakePeer next = peer(1001);
    EXPECT_TRUE(tests::eventually([&] {
        next.send(Connect{.room = 1});
        return next.waitFor<Accept>(std::chrono::milliseconds(10)).has_value();
    }));

    EXPECT_EQ(m_created, 2);
}

TEST_F(RoomRoutingTest, AFailingRoomOnlyDisconnectsItsPlayers)
{
    FakePeer failing = peer(1000);
    FakePeer healthy = peer(1001);
    join(failing, 1);
    join(healthy, 2);

    failing.send(Input{.sequence = 1, .actions = FailAction});

    const auto closed = failing.waitFor<Disconnect>();
    ASSERT_TRUE(closed.has_value());
    EXPECT_EQ(closed->reason, DisconnectReason::ServerClosing);
    healthy.send(Input{.sequence = 1, .actions = 2});
    const auto event = healthy.waitFor<GameEvent>();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->slot, 2);
}
