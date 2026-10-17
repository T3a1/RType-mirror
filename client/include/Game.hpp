/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#ifndef CLIENT_GAME_HPP
#define CLIENT_GAME_HPP

#include "engine/Clock.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkClient.hpp"
#include "systems/NetworkSyncSystem.hpp"

#include <cstdint>
#include <string>

namespace client {

// The client game: its own registry (a mirror of the server's world plus
// local-only entities such as the background), and a loop that runs once
// per displayed frame with a variable time step.
class Game {
  public:
    // Throws if the server address cannot be resolved.
    Game(const std::string &host, std::uint16_t port);
    ~Game();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;
    Game(Game &&) = delete;
    Game &operator=(Game &&) = delete;

    // Runs until the window is closed.
    void run();

  private:
    void update();
    void render();
    void drawStatus() const;

    NetworkClient m_network;
    engine::Registry m_registry;
    SyncState m_sync;
};

} // namespace client

#endif // CLIENT_GAME_HPP
