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
#include "systems/BoundarySystem.hpp"
#include "systems/LifetimeSystem.hpp"
#include "systems/ShootSystem.hpp"
#include "systems/SpawnSystem.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

namespace server {

// using Inbound = engine::MessageQueue<InboundMessage>;
// using Outbound = engine::MessageQueue<OutboundMessage>;

struct Player {
    engine::Entity ship;
    std::uint8_t slot;
    std::uint32_t lastSequence;
};

class Game {
  public:
    void run();
    void stop();
    // Game(Inbound &inbound, Outbound &outbound)
    //     : m_inbound(inbound), m_outbound(outbound)
    // {
    // }

  private:
    // Inbound &m_inbound;
    // Outbound &m_outbound;
    void tick(engine::Duration dt);
    engine::Registry m_registry;
    EntityFactory m_factory;
    std::atomic<bool> m_running = true;
    // std::map<std::uint32_t, Player> m_players;
    std::size_t m_nextNetworkId = 1;
    std::size_t m_tick = 1;
    static_assert(std::atomic<bool>::is_always_lock_free);
};

} // namespace server

#endif // SERVER_GAME_HPP
