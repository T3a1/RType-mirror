/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EnemyAISystem
*/

#include "systems/EnemyAISystem.hpp"

namespace server {

void enemyAISystem(engine::Registry &registry)
{
    registry.each<rtype::Weapon, rtype::Enemy, rtype::Transform>(
        [](engine::Entity, rtype::Weapon &weapon, const rtype::Enemy &,
           const rtype::Transform &transform) {
            weapon.triggered = transform.position.x > 0.0F &&
                               transform.position.x < rtype::PlayfieldWidth;
        });
}

} // namespace server
