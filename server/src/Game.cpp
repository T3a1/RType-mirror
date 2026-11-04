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
}

void Game::tick(engine::Duration dt)
{
    const auto messages = m_inbound.drain();
    server::sessionSystem(m_registry, messages, m_players, m_factory,
                          m_outbound, m_tick);
    server::networkInputSystem(m_registry, messages, m_players);
    server::roomPhaseSystem(messages, m_status, m_outbound);
    if (m_status != rtype::protocol::RoomStatus::Waiting) {
        simulate(dt);
    }
    m_registry.flush();
    if (m_tick % 3 == 0) {
        server::snapshotSystem(m_registry, m_players, m_outbound, m_tick);
    }
    m_publicStatus = static_cast<std::uint8_t>(m_status);
    ++m_tick;
}

void Game::simulate(engine::Duration dt)
{
    rtype::playerFireSystem(m_registry);
    server::enemyAISystem(m_registry);
    server::spawnSystem(m_registry, m_factory, dt);
    server::shootSystem(m_registry, m_factory, dt);
    rtype::playerControlSystem(m_registry);
    rtype::patternSystem(m_registry, dt);
    rtype::movementSystem(m_registry, dt);
    const std::vector<Collision> collisions =
        server::collisionSystem(m_registry);
    server::damageSystem(m_registry, collisions, m_outbound,
                         m_invinciblePlayers);
    server::deathSystem(m_registry, m_outbound);
    server::lifetimeSystem(m_registry, dt, m_outbound);
    server::boundarySystem(m_registry, m_outbound);
    server::gameOverSystem(m_registry, m_players, m_outbound, m_gameOver);
    if (m_gameOver) {
        m_status = rtype::protocol::RoomStatus::Over;
    }
}

} // namespace server
