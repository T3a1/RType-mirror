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
    void spawnTestEntities();

    engine::Registry m_registry;
};

} // namespace client

#endif // CLIENT_GAME_HPP
