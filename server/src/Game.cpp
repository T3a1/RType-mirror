/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Game
*/

#include "Game.hpp"
#include "components/LevelProgress.hpp"

#include <utility>

namespace server {

Game::Game(Inbound &inbound, Outbound &outbound, bool invinciblePlayers,
           std::shared_ptr<const std::vector<Level>> levels,
           WeaponStats weapons)
    : m_inbound(inbound), m_outbound(outbound),
      m_invinciblePlayers(invinciblePlayers), m_weapons(weapons)
{
    const engine::Entity world = m_registry.create();
    m_registry.emplace<rtype::Spawner>(world,
                                       rtype::Spawner{
                                           .minDelay = engine::Duration{2.5F},
                                           .maxDelay = engine::Duration{4.5F},
                                       });
    m_registry.emplace<SpawnQueue>(world);
    if (levels && !levels->empty()) {
        m_registry.emplace<LevelProgress>(
            world, LevelProgress{.levels = std::move(levels)});
    }
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
        Stage::Simulation, "level",
        [this](Registry &r, Duration dt) {
            if (const auto level = server::levelSystem(r, dt)) {
                m_outbound.push(OutboundMessage{
                    .to = Recipient::All,
                    .payload =
                        rtype::protocol::GameEvent{
                            .kind =
                                rtype::protocol::GameEventKind::LevelStarted,
                            .level = *level,
                        },
                });
            }
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "spawn",
        [this](Registry &r, Duration dt) {
            server::spawnSystem(r, m_commands, dt);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "shoot",
        [this](Registry &r, Duration dt) {
            server::shootSystem(r, m_commands, dt, m_weapons);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "spreadShot",
        [this](Registry &r, Duration) {
            server::spreadShotSystem(r, m_commands, m_weapons);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "playerControl",
        [](Registry &r, Duration) { rtype::playerControlSystem(r); }, playing);
    m_scheduler.add(Stage::Simulation, "pattern", rtype::patternSystem,
                    playing);
    m_scheduler.add(
        Stage::Simulation, "boss",
        [this](Registry &r, Duration dt) {
            server::bossSystem(r, m_commands, dt);
        },
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
        Stage::Simulation, "pickup",
        [this](Registry &r, Duration) {
            server::pickupSystem(r, m_collisions, m_outbound,
                                 m_weapons.maxLevel);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "powerUpDrop",
        [this](Registry &r, Duration) {
            server::powerUpDropSystem(r, m_commands);
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

    m_scheduler.add(Stage::Cleanup, "spawnCommands",
                    [this](Registry &r, Duration) {
                        server::spawnCommandSystem(r, m_commands, m_factory);
                    });

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
