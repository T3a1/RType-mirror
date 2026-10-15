/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Game
*/

#include "Game.hpp"

namespace server {

void Game::tick(engine::Duration dt)
{
    server::spawnSystem(m_registry, m_factory, dt);
    rtype::movementSystem(m_registry, dt);
    server::boundarySystem(m_registry);

    // Later, in this order: input, AI, collision, damage, death,
    // spawn, snapshot.
    m_registry.flush();
    // ++m_tick;
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
