/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** main
*/

#include "Game.hpp"
#include "common/protocol/Connection.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

constexpr int ExitError = 84;

constexpr const char *Usage =
    "Usage: r-type_client [host] [port] [--no-interpolation]\n"
    "                     [--no-prediction] [--room N]\n"
    "                     [--bench SECONDS FILE.csv]\n";

/**
 * @struct Arguments
 * @brief The parsed command line of the client.
 * @param host The server's host name or IP address.
 * @param port The server's UDP port.
 * @param options The game options.
 **/
struct Arguments {
    std::string host = "127.0.0.1";
    int port = rtype::protocol::DefaultPort;
    client::GameOptions options;
};

/**
 * @brief Parses the command line: up to two positional arguments (host,
 * port), the room and the benchmark options.
 * @param args The arguments, without the program name.
 * @return The parsed arguments.
 * @throws std::invalid_argument On an unknown option, a missing value or
 * an invalid number.
 **/
Arguments parseArguments(std::span<char *> args)
{
    Arguments parsed;
    std::vector<std::string> positional;
    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string_view arg = args[i];
        if (arg == "--no-interpolation") {
            parsed.options.interpolate = false;
        } else if (arg == "--no-prediction") {
            parsed.options.predict = false;
        } else if (arg == "--room") {
            if (i + 1 >= args.size()) {
                throw std::invalid_argument("--room needs a number");
            }
            const long long room = std::stoll(args[i + 1]);
            if (room < 1 || !std::in_range<std::uint32_t>(room)) {
                throw std::invalid_argument("invalid --room number");
            }
            parsed.options.room = static_cast<std::uint32_t>(room);
            ++i;
        } else if (arg == "--bench") {
            if (i + 2 >= args.size()) {
                throw std::invalid_argument("--bench needs SECONDS FILE");
            }
            const float seconds = std::stof(args[i + 1]);
            if (!(seconds > 0.0F)) {
                throw std::invalid_argument("--bench needs SECONDS > 0");
            }
            parsed.options.bench = client::BenchConfig{
                .duration = engine::Duration{seconds},
                .output = args[i + 2],
            };
            i += 2;
        } else if (arg.starts_with("--")) {
            throw std::invalid_argument("unknown option " + std::string{arg});
        } else {
            positional.emplace_back(arg);
        }
    }
    if (positional.size() > 2) {
        throw std::invalid_argument("too many arguments");
    }
    if (!positional.empty()) {
        parsed.host = positional[0];
    }
    if (positional.size() == 2) {
        parsed.port = std::stoi(positional[1]);
    }
    return parsed;
}

} // namespace

int main(int argc, char *argv[])
{
    const std::span args(argv, static_cast<std::size_t>(argc));
    Arguments parsed;
    try {
        parsed = parseArguments(args.subspan(1));
    } catch (const std::exception &error) {
        std::cerr << "r-type_client: " << error.what() << '\n' << Usage;
        return ExitError;
    }
    if (parsed.port <= 0 || parsed.port > UINT16_MAX) {
        std::cerr << "Invalid port: " << parsed.port << '\n';
        return ExitError;
    }
    try {
        client::Game game(parsed.host, static_cast<std::uint16_t>(parsed.port),
                          parsed.options);
        game.run();
    } catch (const std::exception &error) {
        std::cerr << "r-type_client: " << error.what() << '\n';
        return ExitError;
    }
    return 0;
}
