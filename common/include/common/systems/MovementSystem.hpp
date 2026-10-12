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

// Moves every entity with a Transform and a Velocity by velocity * dt.
void movementSystem(engine::Registry &registry, engine::Duration dt);

} // namespace rtype

#endif // COMMON_SYSTEMS_MOVEMENTSYSTEM_HPP
