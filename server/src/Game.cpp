/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Game
*/

#include "Game.hpp"

namespace server {

/**
 * Registers every system of the game in the scheduler, stage by stage and
 * in the order they run within a stage.
 **/
void Game::registerSystems()
{
    using engine::Duration;
    using engine::Registry;
    using engine::Stage;

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
    m_scheduler.add(Stage::Input, "playerFire",
                    [](Registry &r, Duration) { rtype::playerFireSystem(r); });
    m_scheduler.add(Stage::Input, "enemyAI",
                    [](Registry &r, Duration) { server::enemyAISystem(r); });

    m_scheduler.add(Stage::Simulation, "spawn",
                    [this](Registry &r, Duration dt) {
                        server::spawnSystem(r, m_factory, dt);
                    });
    m_scheduler.add(Stage::Simulation, "shoot",
                    [this](Registry &r, Duration dt) {
                        server::shootSystem(r, m_factory, dt);
                    });
    m_scheduler.add(
        Stage::Simulation, "playerControl",
        [](Registry &r, Duration) { rtype::playerControlSystem(r); });
    m_scheduler.add(Stage::Simulation, "movement", rtype::movementSystem);
    m_scheduler.add(Stage::Simulation, "collision",
                    [this](Registry &r, Duration) {
                        m_collisions = server::collisionSystem(r);
                    });
    m_scheduler.add(Stage::Simulation, "damage", [this](Registry &r, Duration) {
        server::damageSystem(r, m_collisions, m_outbound);
    });
    m_scheduler.add(Stage::Simulation, "death", [this](Registry &r, Duration) {
        server::deathSystem(r, m_outbound);
    });
    m_scheduler.add(Stage::Simulation, "lifetime",
                    [this](Registry &r, Duration dt) {
                        server::lifetimeSystem(r, dt, m_outbound);
                    });
    m_scheduler.add(Stage::Simulation, "boundary",
                    [this](Registry &r, Duration) {
                        server::boundarySystem(r, m_outbound);
                    });

    m_scheduler.add(
        Stage::Output, "snapshot",
        [this](Registry &r, Duration) {
            server::snapshotSystem(r, m_players, m_outbound, m_tick);
        },
        3);
}

void Game::tick(engine::Duration dt)
{
    m_scheduler.run(m_registry, dt);
    ++m_tick;
}

void Game::run()
{
    using Clock = std::chrono::steady_clock;
    constexpr Clock::duration tickStep =
        std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(1.0 / 60.0));
    constexpr Clock::duration maxCatchUp = std::chrono::milliseconds(250);

    Clock::duration accumulator{0};
    Clock::time_point previous = Clock::now();

    const engine::Entity world = m_registry.create();
    m_registry.emplace<rtype::Spawner>(world);

    while (m_running) {
        const Clock::time_point now = Clock::now();
        accumulator = std::min(accumulator + (now - previous), maxCatchUp);
        previous = now;

        while (accumulator >= tickStep) {
            tick(engine::Duration{tickStep});
            accumulator -= tickStep;
        }

        std::this_thread::sleep_until(now + (tickStep - accumulator));
    }
}

void Game::stop() { m_running = false; }

} // namespace server
