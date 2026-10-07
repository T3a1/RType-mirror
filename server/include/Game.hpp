/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Game
*/

#ifndef SERVER_GAME_HPP
#define SERVER_GAME_HPP
#include "engine/Clock.hpp"
#include "engine/Registry.hpp"
#include <atomic>

namespace server {
class Game {
  public:
    void run();
    void stop();

  private:
    void tick(engine::Duration dt);
    engine::Registry m_registry;
    std::atomic<bool> m_running = true;

    static_assert(std::atomic<bool>::is_always_lock_free);
};

} // namespace server

#endif // SERVER_GAME_HPP
