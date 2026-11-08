/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnCommandSystem
*/

#include "systems/SpawnCommandSystem.hpp"

namespace server {

void spawnCommandSystem(engine::Registry &registry, SpawnCommands &commands,
                        EntityFactory &factory)
{
    for (const SpawnCommand &command : commands) {
        std::visit(
            [&](const auto &request) { factory.create(registry, request); },
            command);
    }
    commands.clear();
}

} // namespace server
