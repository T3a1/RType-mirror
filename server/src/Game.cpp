/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Game
*/

#include "Game.hpp"
#include "common/components/Health.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "systems/MovementSystem.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

namespace server {

void Game::tick(engine::Duration dt)
{
    movementSystem(m_registry, dt);
    // Later, in this order: input, AI, collision, damage, death,
    // spawn, snapshot.

    m_registry.each<rtype::Transform>([](engine::Entity, rtype::Transform &t) {
        std::cout << "x = " << t.position.x << '\n';
    });

    // Always last: removes the entities destroyed during this tick.
    m_registry.flush();
}

void Game::init_registry()
{
    engine::Entity test = m_registry.create();
    engine::Entity test2 = m_registry.create();
    m_registry.emplace<rtype::Health>(test, rtype::Health{});
    m_registry.emplace<rtype::Transform>(
        test2, rtype::Transform{.position = {0.0, 0.0}});
    // m_registry.emplace<rtype::Velocity>(test2, rtype::Velocity{.value =
    // {10.0, 0.0}});
    std::cout << "[INFO] Registry initialized\n";
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

    init_registry();

    while (m_running) {
        const Clock::time_point now = Clock::now();
        accumulator = std::min(accumulator + (now - previous), maxCatchUp);
        previous = now;

        std::cout << accumulator << " >= " << tickStep << "?\n";
        while (accumulator >= tickStep) {
            tick(engine::Duration{tickStep});
            accumulator -= tickStep;
        }

        std::this_thread::sleep_until(now + (tickStep - accumulator));
    }
}

void Game::stop() { m_running = false; }

} // namespace server
