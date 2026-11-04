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
#include <chrono>
#include <cstdint>
#include <exception>
#include <future>
#include <iostream>
#include <memory>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

#include <csignal>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

namespace {

inline constexpr std::uint8_t ExitError = 84;
inline constexpr std::uint8_t ExitStatus = 0;
inline constexpr std::chrono::milliseconds StopPollInterval{50};
inline constexpr std::chrono::seconds CommandTimeout{2};
inline constexpr std::size_t MaxPortDigits = 5;
inline constexpr std::string_view Usage =
    "Usage: r-type_server [port] [--banlist FILE] [--whitelist FILE]\n"
    "                     [--no-console] [--invincible]\n";
inline constexpr std::string_view Prompt = "rtype> ";
// Back to the start of the line, then erase it: removes the prompt or the ^C.
inline constexpr std::string_view ClearLine = "\r\033[K";
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
volatile std::sig_atomic_t g_stopRequested = 0;

// Only async-signal-safe work here: no logging, no allocation.
void onSignal(int signal)
{
    static_cast<void>(std::signal(signal, SIG_DFL));
    g_stopRequested = 1;
}

/**
 * @struct Options
 * @brief The command line of the server.
 * @param port The UDP port to listen on.
 * @param invincible Whether the players' ships take no damage.
 * @param console Whether to read commands from the standard input.
 * @param banFile The file of the banned addresses, if any.
 * @param whitelistFile The file of the allowed addresses, if any; it turns
 * the white list on.
 **/
struct Options {
    std::uint16_t port = rtype::protocol::DefaultPort;
    bool invincible = false;
    bool console = true;
    std::string banFile;
    std::string whitelistFile;
};

/**
 * @brief Reads the port argument.
 * @param text The argument.
 * @return The port.
 * @throws std::invalid_argument If it is not a number from 1 to 65535.
 **/
std::uint16_t parsePort(const std::string &text)
{
    if (text.empty() || text.size() > MaxPortDigits ||
        text.find_first_not_of("0123456789") != std::string::npos) {
        throw std::invalid_argument("invalid port: " + text);
    }
    const int port = std::stoi(text);
    if (port < 1 || port > UINT16_MAX) {
        throw std::invalid_argument("invalid port: " + text);
    }
    return static_cast<std::uint16_t>(port);
}

/**
 * @brief Parses the command line.
 * @param args The arguments, without the program name.
 * @return The options.
 * @throws std::invalid_argument On an unknown option, a missing value, an
 * invalid port or too many arguments.
 **/
Options parseOptions(std::span<char *> args)
{
    Options options;
    bool hasPort = false;
    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        if (arg == "--invincible") {
            options.invincible = true;
        } else if (arg == "--no-console") {
            options.console = false;
        } else if (arg == "--banlist" || arg == "--whitelist") {
            if (i + 1 >= args.size()) {
                throw std::invalid_argument(std::string{arg} + " needs a file");
            }
            (arg == "--banlist" ? options.banFile : options.whitelistFile) =
                args[++i];
        } else if (arg.starts_with("--")) {
            throw std::invalid_argument("unknown option " + std::string{arg});
        } else if (hasPort) {
            throw std::invalid_argument("too many arguments");
        } else {
            options.port = parsePort(std::string{arg});
            hasPort = true;
        }
    }
    return options;
}

/**
 * @brief Tells whether a person types in the standard input.
 * @return True on a terminal, where a prompt makes sense.
 **/
bool isInteractive()
{
#if defined(__unix__) || defined(__APPLE__)
    return isatty(STDIN_FILENO) == 1 && isatty(STDERR_FILENO) == 1;
#else
    return false;
#endif
}

/**
 * @brief Reads commands from the standard input, gives them to the network
 * thread and prints the answers, until the input ends. It stays blocked on the
 * input at shutdown, so its thread is detached and only shares the queue.
 * Everything goes through the logger, so that an answer and a log line never
 * mix.
 * @param queue The queue of the server.
 * @param prompt What to show when waiting for a command.
 **/
void runConsole(const std::shared_ptr<server::AdminQueue> &queue,
                const std::string &prompt)
{
    auto &logger = engine::Logger::instance();
    logger.print(prompt);
    for (std::string line; std::getline(std::cin, line);) {
        auto answer = server::submitCommand(*queue, line);
        std::string text = "The server did not answer";
        if (answer.wait_for(CommandTimeout) == std::future_status::ready) {
            text = answer.get();
        }
        if (!text.empty()) {
            text += '\n';
        }
        text += prompt;
        logger.print(text);
    }
    logger.setConsoleFrame({}, {});
}

/**
 * @brief Starts the console once the logs of the startup are written, with a
 * prompt on a terminal. From then on, log lines are written above the prompt.
 * @param queue The queue of the server.
 **/
void startConsole(const std::shared_ptr<server::AdminQueue> &queue)
{
    auto &logger = engine::Logger::instance();
    logger.flush();
    std::string prompt;
    if (isInteractive()) {
        prompt = Prompt;
        logger.setConsoleFrame(std::string{ClearLine}, prompt);
        logger.print("\nAdministration console: type help for the commands, "
                     "quit to stop the server.\n");
    }
    std::thread(runConsole, queue, prompt).detach();
}

} // namespace

int main(int argc, char *argv[])
{
    const std::span args(argv, static_cast<std::size_t>(argc));
    Options options;
    server::AccessList access;
    try {
        options = parseOptions(args.subspan(1));
        access = server::AccessList(options.banFile, options.whitelistFile);
    } catch (const std::exception &error) {
        std::cerr << "r-type_server: " << error.what() << '\n' << Usage;
        return ExitError;
    }
    const bool invincible = options.invincible;
    engine::Logger::instance().start(engine::LogLevel::Info);

    server::Rooms rooms(
        [invincible](server::Rooms::Link &link) {
            return std::make_unique<server::Game>(link.inbound, link.outbound,
                                                  invincible);
        },
        {
            .maxInstances = rtype::protocol::MaxRooms,
            .maxMembers = rtype::protocol::MaxPlayers,
        },
        server::Game::TickStep);
    std::unique_ptr<server::NetworkServer> network;
    try {
        network = std::make_unique<server::NetworkServer>(
            rtype::network::openUdpSocket(options.port), rooms,
            std::move(access));
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

    network->commands().add("quit", "", "Stop the server.",
                            [](const engine::CommandConsole::Args &) {
                                g_stopRequested = 1;
                                return std::string{"Stopping"};
                            });
#ifdef SIGTTIN
    // Started in the background of a shell, the console must not stop the
    // whole server by reading the terminal: the read just fails.
    static_cast<void>(std::signal(SIGTTIN, SIG_IGN));
#endif
    network->start();
    BB_INFO("Server running on UDP port " << options.port
                                          << ", press Ctrl+C to stop");
    if (options.console) {
        startConsole(network->adminQueue());
    }
    while (g_stopRequested == 0) {
        std::this_thread::sleep_for(StopPollInterval);
    }
    engine::Logger::instance().setConsoleFrame(
        isInteractive() && options.console ? std::string{ClearLine}
                                           : std::string{},
        {});
    network->stop();
    rooms.stopAll();
    BB_INFO("Server stopped");
    engine::Logger::instance().stop();
    return ExitStatus;
}
