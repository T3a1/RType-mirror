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
 * Builds the shot of an enemy, straight to the left.
 * @param shooter The entity that fires.
 * @param transform The position of the shooter.
 * @param stats The weapon stats.
 * @return The spawn request of the projectile.
 **/
SpawnProjectile enemyShot(engine::Entity shooter,
                          const rtype::Transform &transform,
                          const WeaponStats &stats)
{
    return {
        .owner = shooter,
        .position = {.x = transform.position.x - MuzzleOffset,
                     .y = transform.position.y},
        .velocity = {.x = -stats.enemyShotSpeed, .y = 0.0F},
        .fromPlayer = false,
    };
}

/**
 * Builds the charged shot of a player.
 * @param shooter The player's ship.
 * @param transform Its position.
 * @param stats The weapon stats.
 * @return The spawn request of the charged shot.
 **/
SpawnProjectile chargedShot(engine::Entity shooter,
                            const rtype::Transform &transform,
                            const WeaponStats &stats)
{
    return {
        .owner = shooter,
        .position = {.x = transform.position.x + (MuzzleOffset * 2.0F),
                     .y = transform.position.y},
        .velocity = {.x = stats.beamSpeed, .y = 0.0F},
        .damage = stats.beamDamage,
        .charged = true,
    };
}

} // namespace

void shootSystem(engine::Registry &registry, SpawnCommands &commands,
                 engine::Duration dt, const WeaponStats &stats)
{
    registry.each<rtype::Weapon, rtype::Transform>(
        [&](engine::Entity entity, rtype::Weapon &weapon,
            const rtype::Transform &transform) {
            weapon.cooldown =
                std::max(weapon.cooldown - dt, engine::Duration{0.0F});
            if (!registry.has<rtype::Player>(entity)) {
                if (weapon.triggered && weapon.cooldown.count() <= 0.0F) {
                    commands.emplace_back(enemyShot(entity, transform, stats));
                    weapon.cooldown += weapon.fireInterval;
                }
                return;
            }
            weapon.fired = false;
            if (!weapon.triggered) {
                // Released: a charged shot if the key was held long enough.
                if (weapon.charge >= stats.chargeTime) {
                    commands.emplace_back(
                        chargedShot(entity, transform, stats));
                }
                weapon.charge = engine::Duration{0.0F};
                return;
            }
            // Pressed: one shot, made by the shot style's system. Held: only
            // charges, no more shots.
            const bool pressed = weapon.charge.count() == 0.0F;
            weapon.charge += dt;
            weapon.fired = pressed && weapon.cooldown.count() <= 0.0F;
            if (weapon.fired) {
                weapon.cooldown = stats.fireInterval;
            }
        });
}

} // namespace server
