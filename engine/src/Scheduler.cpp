/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Scheduler
*/

#include "engine/Scheduler.hpp"

namespace engine {

void Scheduler::add(Stage stage, std::string name, System system,
                    SystemOptions options)
{
    m_systems.push_back(Entry{
        .stage = stage,
        .name = std::move(name),
        .system = std::move(system),
        .every = options.every == 0 ? 1 : options.every,
        .when = std::move(options.when),
    });
}

void Scheduler::run(Registry &registry, Duration dt)
{
    for (const Stage stage : STAGE_ORDER) {
        for (const Entry &entry : m_systems) {
            if (entry.stage == stage && m_tick % entry.every == 0 &&
                (!entry.when || entry.when())) {
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
