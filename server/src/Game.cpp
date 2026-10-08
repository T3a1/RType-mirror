/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Game
*/

#include "Game.hpp"
#include "systems/MovementSystem.hpp"
#include <algorithm>
#include <chrono>
#include <thread>

namespace server {

void Game::tick(engine::Duration dt)
{
    movementSystem(m_registry, dt);
    // Later, in this order: input, AI, collision, damage, death,
    // spawn, snapshot.

    m_registry.flush();
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
