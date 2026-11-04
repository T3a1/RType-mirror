/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** main
*/

#include "Game.hpp"
#include "common/network/UdpSocket.hpp"
#include "common/protocol/Connection.hpp"
#include "engine/Logger.hpp"
#include "engine/MessageQueue.hpp"
#include "engine/network/IUdpSocket.hpp"
#include "network/NetworkMessages.hpp"
#include "network/NetworkServer.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <thread>

#include <csignal>

namespace {

inline constexpr std::uint8_t ExitError = 84;
inline constexpr std::uint8_t ExitStatus = 0;
inline constexpr std::chrono::milliseconds StopPollInterval{50};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
volatile std::sig_atomic_t g_stopRequested = 0;

// Only async-signal-safe work here: no logging, no allocation.
void onSignal(int signal)
{
    static_cast<void>(std::signal(signal, SIG_DFL));
    g_stopRequested = 1;
}

} // namespace

int main(int argc, char *argv[])
{
    const std::span args(argv, static_cast<std::size_t>(argc));
    const bool invincible = std::ranges::any_of(args, [](const char *arg) {
        return std::string_view{arg} == "--invincible";
    });
    engine::Logger::instance().start(engine::LogLevel::Info);

    server::Rooms rooms(
        [invincible](server::Rooms::Link &link) {
            return std::make_unique<server::Game>(link.inbound, link.outbound,
                                                  invincible);
        },
        {
            .maxInstances = server::MaxRooms,
            .maxMembers = rtype::protocol::MaxPlayers,
        },
        server::Game::TickStep);
    std::unique_ptr<server::NetworkServer> network;
    try {
        network = std::make_unique<server::NetworkServer>(
            rtype::network::openUdpSocket(rtype::protocol::DefaultPort), rooms);
    } catch (const engine::net::NetworkError &e) {
        BB_FATAL("Could not start server. " << e.what());
        return ExitError;
    }

    if (invincible) {
        BB_INFO("Players are invincible (--invincible)");
    }
    if (std::signal(SIGINT, onSignal) == SIG_ERR ||
        std::signal(SIGTERM, onSignal) == SIG_ERR) {
        BB_FATAL("Failed to install the signal handlers");
        return ExitError;
    }

    network->start();
    BB_INFO("Server running, press Ctrl+C to stop");
    while (g_stopRequested == 0) {
        std::this_thread::sleep_for(StopPollInterval);
    }
    network->stop();
    rooms.stopAll();
    BB_INFO("Server stopped");
    engine::Logger::instance().stop();
    return ExitStatus;
}
