/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** main
*/

#include "Game.hpp"

#include <csignal>
#include <iostream>

namespace {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
server::Game *g_game = nullptr;

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
    server::Game game;
    if (std::signal(SIGINT, onSignal) == SIG_ERR ||
        std::signal(SIGTERM, onSignal) == SIG_ERR) {
        std::cerr << "Failed to install the signal handlers" << '\n';
        return 1;
    }

    g_game = &game;
    std::cout << "Server running, press Ctrl+C to stop" << '\n';
    game.run();
    g_game = nullptr;
    std::cout << "Server stopped" << '\n';
    return 0;
}
