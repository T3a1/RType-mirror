/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** udp_socket_test
*/

#include "common/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/network/IUdpSocket.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <map>
#include <span>
#include <thread>

using namespace std::chrono_literals;
using engine::net::Endpoint;

namespace {

constexpr std::uint32_t Loopback = 0x7F000001;

} // namespace

TEST(Endpoint, FormatsAsDottedAddressAndPort)
{
    EXPECT_EQ((Endpoint{.address = Loopback, .port = 4242}).toString(),
              "127.0.0.1:4242");
    EXPECT_EQ((Endpoint{.address = 0xC0A80A01, .port = 0}).toString(),
              "192.168.10.1:0");
}

TEST(Endpoint, KeysAMapByAddressThenPort)
{
    std::map<Endpoint, int> sessions;
    sessions[{.address = Loopback, .port = 2}] = 1;
    sessions[{.address = Loopback, .port = 1}] = 2;
    sessions[{.address = Loopback, .port = 2}] = 3;

    EXPECT_EQ(sessions.size(), 2U);
    EXPECT_EQ(sessions.begin()->second, 2);
    EXPECT_LT((Endpoint{.address = 1, .port = 9}),
              (Endpoint{.address = 2, .port = 0}));
}

TEST(UdpSocket, ResolvesAnIpv4Address)
{
    EXPECT_EQ(rtype::network::resolve("127.0.0.1", 4242),
              (Endpoint{.address = Loopback, .port = 4242}));
}

TEST(UdpSocket, ThrowsItsOwnErrorForABadHost)
{
    EXPECT_THROW(static_cast<void>(rtype::network::resolve("not a host", 1)),
                 engine::net::NetworkError);
}

TEST(UdpSocket, ExchangesDatagramsOverLoopback)
{
    const auto server = rtype::network::openUdpSocket();
    const auto client = rtype::network::openUdpSocket();
    const Endpoint serverEndpoint{.address = Loopback,
                                  .port = server->localEndpoint().port};
    const std::array<std::uint8_t, 4> sent{1, 2, 3, 4};
    std::array<std::uint8_t, 16> buffer{};

    EXPECT_FALSE(client->sendTo(serverEndpoint, sent));
    const auto datagram =
        server->receive(buffer, 1s).value_or(engine::net::Datagram{});

    ASSERT_EQ(datagram.size, sent.size());
    EXPECT_EQ(datagram.sender.port, client->localEndpoint().port);
    EXPECT_TRUE(std::equal(sent.begin(), sent.end(), buffer.begin()));
}

TEST(UdpSocket, ReceiveReturnsNothingAfterItsTimeout)
{
    const auto socket = rtype::network::openUdpSocket();
    std::array<std::uint8_t, 16> buffer{};
    const auto start = std::chrono::steady_clock::now();

    EXPECT_FALSE(socket->receive(buffer, 0ms).has_value());
    EXPECT_FALSE(socket->receive(buffer, 20ms).has_value());

    EXPECT_GE(std::chrono::steady_clock::now() - start, 20ms);
}

TEST(UdpSocket, ReceiveSkipsTheResetOfAClosedPort)
{
    const auto socket = rtype::network::openUdpSocket();
    const auto peer = rtype::network::openUdpSocket();
    std::uint16_t closedPort = 0;
    {
        const auto closed = rtype::network::openUdpSocket();
        closedPort = closed->localEndpoint().port;
    }
    const std::array<std::uint8_t, 4> sent{1, 2, 3, 4};
    std::array<std::uint8_t, 16> buffer{};

    EXPECT_FALSE(
        socket->sendTo({.address = Loopback, .port = closedPort}, sent));
    std::this_thread::sleep_for(50ms);
    EXPECT_FALSE(peer->sendTo(
        {.address = Loopback, .port = socket->localEndpoint().port}, sent));
    std::this_thread::sleep_for(50ms);
    const auto datagram = socket->receive(buffer, 0ms);

    ASSERT_TRUE(datagram.has_value());
    if (datagram) {
        EXPECT_EQ(datagram->sender.port, peer->localEndpoint().port);
    }
}

TEST(UdpSocket, ThrowsItsOwnErrorForAPortInUse)
{
    const auto first = rtype::network::openUdpSocket();
    const std::uint16_t port = first->localEndpoint().port;

    EXPECT_THROW(static_cast<void>(rtype::network::openUdpSocket(port)),
                 engine::net::NetworkError);
}
