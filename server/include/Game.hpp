/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Game
*/

#ifndef SERVER_GAME_HPP
#define SERVER_GAME_HPP

#include "EntityFactory.hpp"
#include "Level.hpp"
#include "Weapons.hpp"
#include "common/components/Spawner.hpp"
#include "common/systems/MovementSystem.hpp"
#include "common/systems/PatternSystem.hpp"
#include "common/systems/PlayerControlSystem.hpp"
#include "engine/Clock.hpp"
#include "engine/IInstance.hpp"
#include "engine/Registry.hpp"
#include "engine/Scheduler.hpp"
#include "network/NetworkMessages.hpp"
#include "systems/BossSystem.hpp"
#include "systems/BoundarySystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/DeathSystem.hpp"
#include "systems/EnemyAISystem.hpp"
#include "systems/GameOverSystem.hpp"
#include "systems/LevelSystem.hpp"
#include "systems/LifetimeSystem.hpp"
#include "systems/NetworkInputSystem.hpp"
#include "systems/PowerUpSystem.hpp"
#include "systems/RoomPhaseSystem.hpp"
#include "systems/SessionSystem.hpp"
#include "systems/ShootSystem.hpp"
#include "systems/ShotStyleSystems.hpp"
#include "systems/SnapshotSystem.hpp"
#include "systems/SpawnCommandSystem.hpp"
#include "systems/SpawnSystem.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <thread>
#include <vector>

namespace server {

/**
 * @class Game
 * @brief The authoritative game simulation of one room, run by the engine's
 * InstanceManager on its own thread.
 *
 * The game owns the registry, the entity factory and the connected players.
 * It talks to the network thread only through the inbound and outbound
 * queues, and registers its systems in a scheduler that runs them once per
 * fixed 60 Hz tick.
 **/
class Game final : public engine::IInstance {
  public:
    /**
     * @brief The time between two ticks.
     **/
    static constexpr std::chrono::steady_clock::duration TickStep =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<double>(1.0 / 60.0));

    /**
     * @brief Runs every system once.
     * @param dt The fixed time step.
     **/
    void tick(engine::Duration dt) override;

    /**
     * @brief Returns whether new players may join.
     * @return False once the game is over; safe from another thread.
     **/
    [[nodiscard]] bool joinable() const override
    {
        return status() !=
               static_cast<std::uint8_t>(rtype::protocol::RoomStatus::Over);
    }

    /**
     * @brief Returns where the game stands.
     * @return A rtype::protocol::RoomStatus; safe from another thread.
     **/
    [[nodiscard]] std::uint8_t status() const override
    {
        return m_publicStatus;
    }

    /**
     * @brief Creates a game connected to the network queues.
     * @param inbound The messages received from the clients.
     * @param outbound The messages to send to the clients.
     * @param invinciblePlayers Whether the players' ships take no damage,
     * for the network benchmark.
     * @param levels The levels played one after the other before the
     * random waves of the endless mode; none for endless mode only.
     * @param weapons The weapon stats, from weapons.json.
     **/
    Game(Inbound &inbound, Outbound &outbound, bool invinciblePlayers = false,
         std::shared_ptr<const std::vector<Level>> levels = {},
         WeaponStats weapons = {});

  private:
    /**
     * @brief Registers every system of the game in the scheduler, stage by
     *stage and in the order they run within a stage. The world only moves once
     *the room has left its waiting phase.
     **/
    void registerSystems();

    Inbound &m_inbound;
    Outbound &m_outbound;
    bool m_invinciblePlayers;
    engine::Registry m_registry;
    engine::Scheduler m_scheduler;
    EntityFactory m_factory;
    WeaponStats m_weapons;
    rtype::protocol::RoomStatus m_status = rtype::protocol::RoomStatus::Waiting;
    std::atomic<std::uint8_t> m_publicStatus{
        static_cast<std::uint8_t>(rtype::protocol::RoomStatus::Waiting)};
    Players m_players;
    bool m_gameOver = false;
    std::uint32_t m_tick = 1;
    std::vector<InboundMessage> m_messages;
    std::vector<Collision> m_collisions;
    SpawnCommands m_commands;
    static_assert(std::atomic<bool>::is_always_lock_free);
};

} // namespace server

#endif // SERVER_GAME_HPP
