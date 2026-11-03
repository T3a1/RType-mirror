/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** main
*/

#include "Controls.hpp"
#include "Game.hpp"
#include "common/protocol/Connection.hpp"

#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr int ExitError = 84;

constexpr const char *Usage =
    "Usage: r-type_client [host] [port] [--controls FILE]\n"
    "                     [--no-interpolation] [--no-prediction]\n"
    "                     [--bench SECONDS FILE.csv]\n";
constexpr const char *DefaultControlsPath = "controls.cfg";

/**
 * @struct Arguments
 * @brief The parsed command line of the client.
 * @param host The server's host name or IP address.
 * @param port The server's UDP port.
 * @param options The game options.
 * @param controlsPath The controls file given with --controls, if any.
 **/
struct Arguments {
    std::string host = "127.0.0.1";
    int port = rtype::protocol::DefaultPort;
    client::GameOptions options;
    std::optional<std::string> controlsPath;
};

/**
 * @brief Parses the command line: up to two positional arguments (host,
 * port) and the benchmark options.
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
        } else if (arg == "--controls") {
            if (i + 1 >= args.size()) {
                throw std::invalid_argument("--controls needs FILE");
            }
            parsed.controlsPath = args[i + 1];
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

/**
 * @brief Reads the controls file over the default controls and reports
 * its bad lines on stderr.
 * @param path The file given with --controls, or nothing to read
 * controls.cfg from the current directory if it exists.
 * @return The controls.
 * @throws std::runtime_error If the file given with --controls cannot be
 * opened.
 **/
client::Controls loadControls(const std::optional<std::string> &path)
{
    client::Controls controls = client::defaultControls();
    const std::string file = path.value_or(DefaultControlsPath);
    std::ifstream in(file);
    if (!in) {
        if (path) {
            throw std::runtime_error("cannot open " + file);
        }
        return controls;
    }
    for (const std::string &error : client::readControls(in, controls)) {
        std::cerr << "r-type_client: " << file << ": " << error << '\n';
    }
    return controls;
}

} // namespace

int main(int argc, char *argv[])
{
    const std::span args(argv, static_cast<std::size_t>(argc));
    Arguments parsed;
    try {
        parsed = parseArguments(args.subspan(1));
        parsed.options.controls = loadControls(parsed.controlsPath);
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
