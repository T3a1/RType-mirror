/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** MovementSystem
*/

#include "common/systems/MovementSystem.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "engine/Entity.hpp"

namespace rtype {

void movementSystem(engine::Registry &registry, engine::Duration dt)
{
    registry.each<Transform, Velocity>(
        [dt](engine::Entity, Transform &transform, Velocity &velocity) {
            transform.position += velocity.value * dt.count();
        });
}

} // namespace rtype
