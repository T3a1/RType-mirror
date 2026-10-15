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

#include <raylib.h>

#include <random>

namespace client {

// The client game: its own registry (a mirror of the server's world plus
// local-only entities such as the background), and a loop that runs once
// per displayed frame with a variable time step.
class Game {
  public:
    Game();
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
    void spawnTestEntities();

    engine::Registry m_registry;
    Texture2D m_backgroundTexture{};
    std::mt19937 m_random{std::random_device{}()};
};

} // namespace client

#endif // CLIENT_GAME_HPP
