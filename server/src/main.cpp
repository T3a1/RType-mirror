/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** main
*/

#include "Game.hpp"
#include "engine/Logger.hpp"
#include "engine/MessageQueue.hpp"
#include "network/NetworkMessages.hpp"
#include "network/NetworkServer.hpp"
#include <cstdint>
#include <memory>

#include <csignal>

namespace {

inline constexpr std::uint8_t ExitError = 84;
inline constexpr std::uint8_t ExitStatus = 0;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
server::Game *g_game = nullptr;

// Only async-signal-safe work here: no logging, no allocation.
void onSignal(int signal)
{
    static_cast<void>(std::signal(signal, SIG_DFL));
    if (g_game != nullptr) {
        g_game->stop();
    }
}

} // namespace

int main(int /*argc*/, char * /*argv*/[])
{
    engine::Logger::instance().start(engine::LogLevel::Debug);

    engine::MessageQueue<server::InboundMessage> inbound(1024);
    engine::MessageQueue<server::OutboundMessage> outbound;
    std::unique_ptr<server::NetworkServer> network;
    try {
        network = std::make_unique<server::NetworkServer>(inbound, outbound);
    } catch (asio::system_error &e) {
        BB_FATAL("Could not start server. " << e.what());
        return ExitError;
    }

    server::Game game;
    if (std::signal(SIGINT, onSignal) == SIG_ERR ||
        std::signal(SIGTERM, onSignal) == SIG_ERR) {
        BB_FATAL("Failed to install the signal handlers");
        return ExitError;
    }

    network->start();
    g_game = &game;
    BB_INFO("Server running, press Ctrl+C to stop");
    game.run();
    g_game = nullptr;
    network->stop();
    BB_INFO("Server stopped");
    engine::Logger::instance().stop();
    return ExitStatus;
}
