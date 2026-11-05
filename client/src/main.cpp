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
#include <utility>
#include <vector>

namespace {

constexpr int ExitError = 84;

constexpr const char *Usage =
    "Usage: r-type_client [host] [port] [--controls FILE]\n"
    "                     [--no-interpolation] [--no-prediction]\n"
    "                     [--room N] [--bench SECONDS FILE.csv]\n";
/** @brief The controls file read if --controls is not given. */
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
 * @brief Returns the value of an option.
 * @param args The arguments.
 * @param index The position of the value.
 * @param option The option, for the error message.
 * @return The value.
 * @throws std::invalid_argument If the value is missing.
 **/
const char *optionValue(std::span<char *> args, std::size_t index,
                        std::string_view option)
{
    if (index >= args.size()) {
        throw std::invalid_argument(std::string{option} + " needs a value");
    }
    return args[index];
}

/**
 * @brief Reads the value of --room.
 * @param text The value.
 * @return The room.
 * @throws std::invalid_argument If it is not a positive 32-bit number.
 **/
std::uint32_t parseRoom(const char *text)
{
    const long long room = std::stoll(text);
    if (room < 1 || !std::in_range<std::uint32_t>(room)) {
        throw std::invalid_argument("invalid --room number");
    }
    return static_cast<std::uint32_t>(room);
}

/**
 * @brief Reads the values of --bench.
 * @param seconds The duration.
 * @param output The CSV file.
 * @return The benchmark configuration.
 * @throws std::invalid_argument If the duration is not positive.
 **/
client::BenchConfig parseBench(const char *seconds, const char *output)
{
    const float duration = std::stof(seconds);
    if (!(duration > 0.0F)) {
        throw std::invalid_argument("--bench needs SECONDS > 0");
    }
    return {.duration = engine::Duration{duration}, .output = output};
}

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
            parsed.options.room = parseRoom(optionValue(args, ++i, arg));
        } else if (arg == "--controls") {
            parsed.controlsPath = optionValue(args, ++i, arg);
        } else if (arg == "--bench") {
            const char *seconds = optionValue(args, ++i, arg);
            parsed.options.bench =
                parseBench(seconds, optionValue(args, ++i, arg));
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

/**
 * @brief Starts the client and runs the game until its window is closed.
 * @param argc The number of command line arguments.
 * @param argv The command line arguments, see the usage text.
 * @return 0 after a normal end, 84 after an error.
 **/
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
