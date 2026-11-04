/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Game
*/

#include "Game.hpp"

namespace server {

Game::Game(Inbound &inbound, Outbound &outbound, bool invinciblePlayers)
    : m_inbound(inbound), m_outbound(outbound),
      m_invinciblePlayers(invinciblePlayers)
{
    const engine::Entity world = m_registry.create();
    m_registry.emplace<rtype::Spawner>(world,
                                       rtype::Spawner{
                                           .minDelay = engine::Duration{2.5F},
                                           .maxDelay = engine::Duration{4.5F},
                                       });
    m_registry.emplace<SpawnQueue>(world);
    registerSystems();
}

/**
 * Registers every system of the game in the scheduler, stage by stage and
 * in the order they run within a stage. The world only moves once the room
 * has left its waiting phase.
 **/
void Game::registerSystems()
{
    using engine::Duration;
    using engine::Registry;
    using engine::Stage;

    const engine::SystemOptions playing{.when = [this] {
        return m_status != rtype::protocol::RoomStatus::Waiting;
    }};

    m_scheduler.add(Stage::Input, "drainInbound", [this](Registry &, Duration) {
        m_messages = m_inbound.drain();
    });
    m_scheduler.add(Stage::Input, "session", [this](Registry &r, Duration) {
        server::sessionSystem(r, m_messages, m_players, m_factory, m_outbound,
                              m_tick);
    });
    m_scheduler.add(Stage::Input, "networkInput",
                    [this](Registry &r, Duration) {
                        server::networkInputSystem(r, m_messages, m_players);
                    });
    m_scheduler.add(Stage::Input, "roomPhase", [this](Registry &, Duration) {
        server::roomPhaseSystem(m_messages, m_status, m_outbound);
    });
    m_scheduler.add(
        Stage::Input, "playerFire",
        [](Registry &r, Duration) { rtype::playerFireSystem(r); }, playing);
    m_scheduler.add(
        Stage::Input, "enemyAI",
        [](Registry &r, Duration) { server::enemyAISystem(r); }, playing);

    m_scheduler.add(
        Stage::Simulation, "spawn",
        [this](Registry &r, Duration dt) {
            server::spawnSystem(r, m_factory, dt);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "shoot",
        [this](Registry &r, Duration dt) {
            server::shootSystem(r, m_factory, dt);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "playerControl",
        [](Registry &r, Duration) { rtype::playerControlSystem(r); }, playing);
    m_scheduler.add(Stage::Simulation, "pattern", rtype::patternSystem,
                    playing);
    m_scheduler.add(Stage::Simulation, "movement", rtype::movementSystem,
                    playing);
    m_scheduler.add(
        Stage::Simulation, "collision",
        [this](Registry &r, Duration) {
            m_collisions = server::collisionSystem(r);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "damage",
        [this](Registry &r, Duration) {
            server::damageSystem(r, m_collisions, m_outbound,
                                 m_invinciblePlayers);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "death",
        [this](Registry &r, Duration) { server::deathSystem(r, m_outbound); },
        playing);
    m_scheduler.add(
        Stage::Simulation, "lifetime",
        [this](Registry &r, Duration dt) {
            server::lifetimeSystem(r, dt, m_outbound);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "boundary",
        [this](Registry &r, Duration) {
            server::boundarySystem(r, m_outbound);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "gameOver",
        [this](Registry &r, Duration) {
            server::gameOverSystem(r, m_players, m_outbound, m_gameOver);
            if (m_gameOver) {
                m_status = rtype::protocol::RoomStatus::Over;
            }
        },
        playing);

    m_scheduler.add(Stage::Output, "snapshot",
                    [this](Registry &r, Duration) {
                        server::snapshotSystem(r, m_players, m_outbound,
                                               m_tick);
                    },
                    {.every = 3});
}

void Game::tick(engine::Duration dt)
{
    m_scheduler.run(m_registry, dt);
    m_publicStatus = static_cast<std::uint8_t>(m_status);
    ++m_tick;
}

} // namespace server
