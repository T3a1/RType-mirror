/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** main
*/

#include "Game.hpp"
#include "Level.hpp"
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
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

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

/**
 * @brief Finds the directory of the level files: the value of --levels, or
 * the levels directory next to the executable.
 * @param args The command line, program name included.
 * @return The directory.
 **/
std::filesystem::path levelsDirectory(std::span<char *> args)
{
    for (std::size_t i = 1; i + 1 < args.size(); ++i) {
        if (std::string_view{args[i]} == "--levels") {
            return args[i + 1];
        }
    }
    const std::filesystem::path program =
        args.empty() ? std::filesystem::path{} : std::filesystem::path{args[0]};
    return program.parent_path() / "levels";
}

/**
 * @brief Loads the levels and logs the files that could not be read.
 * @param directory The directory of the level files.
 * @return The levels, shared by every room; empty for endless mode only.
 **/
std::shared_ptr<const std::vector<server::Level>>
loadLevels(const std::filesystem::path &directory)
{
    server::LevelSet set = server::loadLevels(directory);
    for (const std::string &error : set.errors) {
        BB_WARN("Level skipped: " << error);
    }
    if (set.levels.empty()) {
        BB_WARN("No level in " << directory.string() << ": endless mode only");
    } else {
        BB_INFO(set.levels.size()
                << " level(s) loaded from " << directory.string());
    }
    return std::make_shared<const std::vector<server::Level>>(
        std::move(set.levels));
}

} // namespace

int main(int argc, char *argv[])
{
    const std::span args(argv, static_cast<std::size_t>(argc));
    const bool invincible = std::ranges::any_of(args, [](const char *arg) {
        return std::string_view{arg} == "--invincible";
    });
    engine::Logger::instance().start(engine::LogLevel::Info);
    const auto levels = loadLevels(levelsDirectory(args));

    server::Rooms rooms(
        [invincible, levels](server::Rooms::Link &link) {
            return std::make_unique<server::Game>(link.inbound, link.outbound,
                                                  invincible, levels);
        },
        {
            .maxInstances = rtype::protocol::MaxRooms,
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
