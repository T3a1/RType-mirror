/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** MovementSystem
*/

#ifndef COMMON_SYSTEMS_MOVEMENTSYSTEM_HPP
#define COMMON_SYSTEMS_MOVEMENTSYSTEM_HPP

#include "engine/Clock.hpp"
#include "engine/Registry.hpp"

namespace rtype {

/**
 * @brief Moves every entity with a Transform and a Velocity by velocity * dt.
 *
 * Runs in the Simulation stage, on the server.
 * @param registry The registry holding the entities.
 * @param dt The time elapsed since the last tick, in seconds.
 **/
void movementSystem(engine::Registry &registry, engine::Duration dt);

} // namespace rtype

#endif // COMMON_SYSTEMS_MOVEMENTSYSTEM_HPP
