/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** MovementSystem
*/

#include "systems/MovementSystem.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "engine/Entity.hpp"

namespace server {

void movementSystem(engine::Registry &registry, engine::Duration dt)
{
    registry.each<rtype::Transform, rtype::Velocity>(
        [dt](engine::Entity, rtype::Transform &transform,
             rtype::Velocity &velocity) {
            transform.position += velocity.value * dt.count();
        });
}

} // namespace server
