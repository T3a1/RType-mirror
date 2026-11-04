/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ShootSystem
*/

#include "systems/ShootSystem.hpp"

#include <cmath>

namespace server {

namespace {

// Distance between the center of the shooter and the start of its shot.
constexpr float MuzzleOffset = 40.0F;

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
 * Builds the shots of a player: one at level 1, then a spread of two more
 * shots at each level.
 * @param shooter The player's ship.
 * @param transform Its position.
 * @param level The level of its weapon.
 * @param stats The weapon stats.
 * @param shots Where the spawn requests go.
 **/
void playerShots(engine::Entity shooter, const rtype::Transform &transform,
                 std::uint8_t level, const WeaponStats &stats,
                 std::vector<SpawnProjectile> &shots)
{
    const int side = static_cast<int>(level) - 1;
    for (int i = -side; i <= side; ++i) {
        const float angle = static_cast<float>(i) * stats.spreadAngle;
        shots.push_back({
            .owner = shooter,
            .position = {.x = transform.position.x + MuzzleOffset,
                         .y = transform.position.y},
            .velocity = {.x = std::cos(angle) * stats.shotSpeed,
                         .y = std::sin(angle) * stats.shotSpeed},
            .damage = stats.shotDamage,
        });
    }
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

void shootSystem(engine::Registry &registry, EntityFactory &factory,
                 engine::Duration dt, const WeaponStats &stats)
{
    std::vector<SpawnProjectile> shotRequests;

    registry.each<rtype::Weapon, rtype::Transform>(
        [&](engine::Entity entity, rtype::Weapon &weapon,
            const rtype::Transform &transform) {
            weapon.cooldown =
                std::max(weapon.cooldown - dt, engine::Duration{0.0F});
            if (!registry.has<rtype::Player>(entity)) {
                if (weapon.triggered && weapon.cooldown.count() <= 0.0F) {
                    shotRequests.push_back(enemyShot(entity, transform, stats));
                    weapon.cooldown += weapon.fireInterval;
                }
                return;
            }
            if (!weapon.triggered) {
                // Released: a charged shot if the key was held long enough.
                if (weapon.charge >= stats.chargeTime) {
                    shotRequests.push_back(
                        chargedShot(entity, transform, stats));
                }
                weapon.charge = engine::Duration{0.0F};
                return;
            }
            // Pressed: one shot. Held: only charges, no more shots.
            const bool pressed = weapon.charge.count() == 0.0F;
            weapon.charge += dt;
            if (pressed && weapon.cooldown.count() <= 0.0F) {
                playerShots(entity, transform, weapon.level, stats,
                            shotRequests);
                weapon.cooldown = stats.fireInterval;
            }
        });
    for (const SpawnProjectile &request : shotRequests) {
        factory.create(registry, request);
    }
}

} // namespace server
