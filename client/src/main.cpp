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
#include <span>
#include <string>

namespace {

constexpr int ExitError = 84;

} // namespace

int main(int argc, char *argv[])
{
    const std::span args(argv, static_cast<std::size_t>(argc));
    try {
        const std::string host = args.size() > 1 ? args[1] : "127.0.0.1";
        const int port =
            args.size() > 2 ? std::stoi(args[2]) : rtype::protocol::DefaultPort;
        if (port <= 0 || port > UINT16_MAX) {
            std::cerr << "Invalid port: " << port << '\n';
            return ExitError;
        }
        client::Game game(host, static_cast<std::uint16_t>(port));
        game.run();
    } catch (const std::exception &error) {
        std::cerr << "r-type_client: " << error.what() << '\n';
        return ExitError;
    }
    return 0;
}
