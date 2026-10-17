/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ShootSystem
*/

#include "systems/ShootSystem.hpp"

namespace server {

namespace {

/**
 * Builds the projectile fired by a shooter.
 * @param shooter The entity that fires.
 * @param transform The position of the shooter.
 * @param fromPlayer Whether the shooter is a player.
 * @return The spawn request of the projectile.
 **/
SpawnProjectile shotRequest(engine::Entity shooter,
                            const rtype::Transform &transform, bool fromPlayer)
{
    const float direction = fromPlayer ? 1.0F : -1.0F;

    return {
        .owner = shooter,
        .position =
            {
                .x = transform.position.x + (direction * 40.0F),
                .y = transform.position.y,
            },
        .velocity = {.x = direction * 100.0F, .y = 0.0F},
        .fromPlayer = fromPlayer,
    };
}

} // namespace

/**
 * Processes the shooting of entities.
 * @param registry The entity registry.
 * @param factory The entity factory.
 * @param dt The time since the last update.
 **/
void shootSystem(engine::Registry &registry, EntityFactory &factory,
                 engine::Duration dt)
{
    std::vector<SpawnProjectile> shotRequests;

    registry.each<rtype::Weapon, rtype::Transform>(
        [&](engine::Entity entity, rtype::Weapon &weapon,
            const rtype::Transform &transform) {
            weapon.cooldown =
                std::max(weapon.cooldown - dt, engine::Duration{0.0F});
            if (!weapon.triggered || weapon.cooldown.count() > 0.0F) {
                return;
            }
            shotRequests.push_back(shotRequest(
                entity, transform, registry.has<rtype::Player>(entity)));
            weapon.cooldown += weapon.fireInterval;
        });
    for (const SpawnProjectile &request : shotRequests) {
        factory.create(registry, request);
    }
}

} // namespace server
