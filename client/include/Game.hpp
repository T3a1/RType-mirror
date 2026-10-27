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

#include <raylib.h>

#include <random>

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
    void update(engine::Duration dt);
    void render();
    /**
     * @brief Loads the background texture and creates the entity scrolling it.
     *
     * The texture is loaded from the assets directory next to the executable
     * and stays owned by the Game, which unloads it in its destructor.
     */
    void spawnBackground();
    void drawStatus() const;
    NetworkClient m_network;
    engine::Registry m_registry;
    Texture2D m_backgroundTexture{};
    SyncState m_sync;
};

} // namespace client

#endif // CLIENT_GAME_HPP
