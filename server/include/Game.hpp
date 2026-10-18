/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Game
*/

#ifndef SERVER_GAME_HPP
#define SERVER_GAME_HPP

#include "EntityFactory.hpp"
#include "common/components/Spawner.hpp"
#include "common/systems/MovementSystem.hpp"
#include "common/systems/PlayerControlSystem.hpp"
#include "engine/Clock.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"
#include "systems/BoundarySystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/DeathSystem.hpp"
#include "systems/EnemyAISystem.hpp"
#include "systems/LifetimeSystem.hpp"
#include "systems/NetworkInputSystem.hpp"
#include "systems/SessionSystem.hpp"
#include "systems/ShootSystem.hpp"
#include "systems/SnapshotSystem.hpp"
#include "systems/SpawnSystem.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <thread>

namespace server {

/**
 * @class Game
 * @brief Runs the authoritative game simulation of the server.
 *
 * The game owns the registry, the entity factory and the connected players.
 * It talks to the network thread only through the inbound and outbound
 * queues, and runs every system once per fixed 60 Hz tick.
 **/
class Game {
  public:
    /**
     * @brief Runs the game loop at a fixed 60 Hz tick until stop() is
     * called.
     **/
    void run();

    /**
     * @brief Asks the game loop to stop after the current tick; safe to
     * call from another thread or a signal handler.
     **/
    void stop();

    /**
     * @brief Creates a game connected to the network queues.
     * @param inbound The messages received from the clients.
     * @param outbound The messages to send to the clients.
     **/
    Game(Inbound &inbound, Outbound &outbound)
        : m_inbound(inbound), m_outbound(outbound)
    {
    }

  private:
    Inbound &m_inbound;
    Outbound &m_outbound;
    void tick(engine::Duration dt);
    engine::Registry m_registry;
    EntityFactory m_factory;
    std::atomic<bool> m_running = true;
    Players m_players;
    std::uint32_t m_tick = 1;
    static_assert(std::atomic<bool>::is_always_lock_free);
};

} // namespace server

#endif // SERVER_GAME_HPP
