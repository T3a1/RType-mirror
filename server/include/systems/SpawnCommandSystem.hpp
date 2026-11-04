/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnCommandSystem
*/

#ifndef SERVER_SYSTEMS_SPAWNCOMMANDSYSTEM_HPP
#define SERVER_SYSTEMS_SPAWNCOMMANDSYSTEM_HPP

#include "EntityFactory.hpp"
#include "SpawnRequests.hpp"
#include "engine/Registry.hpp"

#include <variant>

namespace server {

/**
 * @brief Creates every requested entity, then empties the list.
 * @param registry The registry to create the entities in.
 * @param commands The creations requested during the tick.
 * @param factory The factory that creates the entities.
 **/
void spawnCommandSystem(engine::Registry &registry, SpawnCommands &commands,
                        EntityFactory &factory);

} // namespace server

#endif // SERVER_SYSTEMS_SPAWNCOMMANDSYSTEM_HPP
