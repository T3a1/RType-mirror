/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Scheduler
*/

#include "engine/Scheduler.hpp"

namespace engine {

/**
 * Adds a system to a stage.
 * @param stage The stage the system runs in.
 * @param name The name of the system, for logs and debugging.
 * @param system The system to run.
 * @param every Runs the system once every this many ticks.
 **/
void Scheduler::add(Stage stage, std::string name, System system,
                    std::uint32_t every)
{
    m_systems.push_back(Entry{
        .stage = stage,
        .name = std::move(name),
        .system = std::move(system),
        .every = every == 0 ? 1 : every,
    });
}

/**
 * Runs one tick: every stage in order, then flushes the registry after
 * the Cleanup stage.
 * @param registry The registry the systems work on.
 * @param dt The time elapsed since the last tick.
 **/
void Scheduler::run(Registry &registry, Duration dt)
{
    for (const Stage stage : STAGE_ORDER) {
        for (const Entry &entry : m_systems) {
            if (entry.stage == stage && m_tick % entry.every == 0) {
                entry.system(registry, dt);
            }
        }
        if (stage == Stage::Cleanup) {
            registry.flush();
        }
    }
    ++m_tick;
}

} // namespace engine
