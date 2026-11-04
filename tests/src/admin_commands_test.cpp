/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** admin_commands_test
*/

#include "FakeNetwork.hpp"
#include "FakeRooms.hpp"
#include "network/NetworkServer.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <string>

using namespace rtype::protocol;
using engine::net::Endpoint;
using tests::FakeNetwork;
using tests::FakePeer;

namespace {

const Endpoint ServerEndpoint{.address = 0x0A000001, .port = 4242};
const Endpoint ClientEndpoint{.address = 0x0A000002, .port = 1000};

class AdminCommandsTest : public ::testing::Test {
  public:
    FakeNetwork m_network;
    tests::FakeRooms m_fakeRooms;
    server::NetworkServer m_server{m_network.open(ServerEndpoint),
                                   m_fakeRooms.rooms};

    FakePeer peer() { return peerAt(ClientEndpoint); }

    FakePeer peerAt(const Endpoint &endpoint)
    {
        return {m_network, endpoint, ServerEndpoint, Side::Server};
    }

    // Puts a peer in the lobby.
    static void connect(FakePeer &client)
    {
        client.send(Connect{});
        EXPECT_TRUE(client.waitFor<Accept>().has_value());
    }

    // Puts a peer of the lobby in a room.
    static void enter(FakePeer &client)
    {
        client.send(JoinRoom{.room = 1});
        EXPECT_TRUE(client.waitFor<RoomJoined>().has_value());
    }

    // Runs a command on the network thread and returns its answer.
    [[nodiscard]] std::string run(const std::string &line) const
    {
        auto answer = server::submitCommand(*m_server.adminQueue(), line);
        if (answer.wait_for(std::chrono::seconds(2)) !=
            std::future_status::ready) {
            ADD_FAILURE() << "No answer to " << line;
            return {};
        }
        return answer.get();
    }

  protected:
    void SetUp() override { m_server.start(); }
};

bool contains(const std::string &text, const std::string &part)
{
    return text.find(part) != std::string::npos;
}

} // namespace

TEST_F(AdminCommandsTest, StatusCountsTheClients)
{
    FakePeer client = peer();
    connect(client);

    const std::string status = run("status");

    EXPECT_TRUE(contains(status, "1 (1 in the lobby)")) << status;
    EXPECT_TRUE(contains(status, "Access   open, 0 banned")) << status;
}

TEST_F(AdminCommandsTest, RoomsAndPlayersListThem)
{
    FakePeer client = peer();
    connect(client);
    enter(client);

    EXPECT_TRUE(contains(run("rooms"), "1/4"));
    const std::string players = run("players");
    EXPECT_TRUE(contains(players, ClientEndpoint.toString())) << players;
    EXPECT_FALSE(contains(run("players 9"), ClientEndpoint.toString()));
}

TEST_F(AdminCommandsTest, KickDisconnectsAClient)
{
    FakePeer client = peer();
    connect(client);

    EXPECT_EQ(run("kick 1"), "Client 1 kicked");

    EXPECT_EQ(client.waitFor<Disconnect>(),
              Disconnect{.reason = DisconnectReason::Kicked});
    EXPECT_EQ(run("kick 99"), "No client 99");
    EXPECT_EQ(run("kick abc"), "Usage: kick <id>");
}

TEST_F(AdminCommandsTest, BanDisconnectsAndRefusesAnAddress)
{
    FakePeer client = peer();
    connect(client);

    EXPECT_EQ(run("ban 1"), "10.0.0.2 banned, 1 client disconnected");

    EXPECT_TRUE(client.waitFor<Disconnect>().has_value());
    EXPECT_TRUE(contains(run("banlist"), "10.0.0.2"));
    EXPECT_TRUE(tests::eventually(
        [&] { return !contains(run("players"), ClientEndpoint.toString()); }));
    client.send(Connect{});
    EXPECT_EQ(client.waitFor<Reject>(), Reject{.reason = RejectReason::Banned});
}

TEST_F(AdminCommandsTest, UnbanAcceptsAnAddressAgain)
{
    EXPECT_EQ(run("ban 10.0.0.2"), "10.0.0.2 banned, 0 clients disconnected");
    EXPECT_EQ(run("unban 10.0.0.2"), "10.0.0.2 unbanned");
    EXPECT_EQ(run("unban 10.0.0.2"), "10.0.0.2 was not banned");

    FakePeer client = peer();
    connect(client);
}

TEST_F(AdminCommandsTest, TheWhitelistRefusesOtherAddresses)
{
    EXPECT_EQ(run("whitelist on"), "Only listed addresses can connect now");
    FakePeer client = peer();
    client.send(Connect{});
    EXPECT_EQ(client.waitFor<Reject>(),
              Reject{.reason = RejectReason::NotWhitelisted});

    EXPECT_EQ(run("whitelist add 10.0.0.2"), "10.0.0.2 added");
    connect(client);

    EXPECT_TRUE(contains(run("whitelist list"), "10.0.0.2"));
    EXPECT_EQ(run("whitelist nonsense"),
              "Usage: whitelist on|off|list|add <ip>|remove <ip>");
}

TEST_F(AdminCommandsTest, StopRoomSendsThePlayersBackToTheLobby)
{
    FakePeer client = peer();
    connect(client);
    enter(client);

    EXPECT_EQ(run("stop-room 1"), "Room 1 closed, 1 player back in the lobby");

    EXPECT_TRUE(client.waitFor<RoomClosed>().has_value());
    EXPECT_EQ(run("stop-room 1"), "No room 1");
    EXPECT_TRUE(contains(run("status"), "Rooms    0 / 16"));
}

TEST_F(AdminCommandsTest, AnotherCommandCanBeAdded)
{
    m_server.stop();
    m_server.commands().add(
        "hello", "", "Say hello.",
        [](const engine::CommandConsole::Args &) { return std::string{"hi"}; });
    m_server.start();

    EXPECT_EQ(run("hello"), "hi");
}
