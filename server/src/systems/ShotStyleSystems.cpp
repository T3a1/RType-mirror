/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ShotStyleSystems
*/

#include "systems/ShotStyleSystems.hpp"
#include "SpawnRequests.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Weapon.hpp"
#include "components/ShotStyles.hpp"
#include "engine/Entity.hpp"

#include <cmath>
#include <limits>
#include <optional>
#include <vector>

namespace server {

namespace {

/**
 * @brief Finds the direction from a point to the nearest enemy.
 * @param registry The registry holding the enemies.
 * @param from The point.
 * @return The unit direction, or straight right if there is no enemy.
 **/
engine::Vec2f towardsNearestEnemy(engine::Registry &registry,
                                  engine::Vec2f from)
{
    std::optional<engine::Vec2f> nearest;
    float best = std::numeric_limits<float>::max();
    registry.each<rtype::Enemy, rtype::Transform>(
        [&](engine::Entity, const rtype::Enemy &,
            const rtype::Transform &transform) {
            const engine::Vec2f offset = transform.position - from;
            const float distance = std::hypot(offset.x, offset.y);
            if (distance > 0.0F && distance < best) {
                best = distance;
                nearest = offset * (1.0F / distance);
            }
        });
    return nearest.value_or(engine::Vec2f{.x = 1.0F, .y = 0.0F});
}

} // namespace

void burstShotSystem(engine::Registry &registry, engine::Duration dt,
                     const WeaponStats &stats)
{
    registry.each<rtype::Weapon, BurstShot>(
        [&](engine::Entity, rtype::Weapon &weapon, BurstShot &burst) {
            if (weapon.fired) {
                burst.pending = stats.burstShots;
                burst.delay = stats.burstDelay;
                return;
            }
            if (burst.pending == 0) {
                return;
            }
            burst.delay -= dt;
            if (burst.delay.count() > 0.0F) {
                return;
            }
            weapon.fired = true;
            --burst.pending;
            burst.delay = stats.burstDelay;
        });
}

void spreadShotSystem(engine::Registry &registry, SpawnCommands &commands,
                      const WeaponStats &stats)
{
    registry.each<rtype::Weapon, SpreadShot, rtype::Transform>(
        [&](engine::Entity entity, const rtype::Weapon &weapon,
            const SpreadShot &spread, const rtype::Transform &transform) {
            if (!weapon.fired) {
                return;
            }
            const int side = static_cast<int>(spread.level) - 1;
            for (int i = -side; i <= side; ++i) {
                const float angle = static_cast<float>(i) * stats.spreadAngle;
                commands.emplace_back(SpawnProjectile{
                    .owner = entity,
                    .position = {.x = transform.position.x + MuzzleOffset,
                                 .y = transform.position.y},
                    .velocity = {.x = std::cos(angle) * stats.shotSpeed,
                                 .y = std::sin(angle) * stats.shotSpeed},
                    .damage = stats.shotDamage,
                });
            }
        });
}

void aimedShotSystem(engine::Registry &registry, SpawnCommands &commands,
                     const WeaponStats &stats)
{
    registry.each<rtype::Weapon, AimedShot, rtype::Transform>(
        [&](engine::Entity entity, const rtype::Weapon &weapon,
            const AimedShot &, const rtype::Transform &transform) {
            if (!weapon.fired) {
                return;
            }
            const engine::Vec2f muzzle{.x = transform.position.x + MuzzleOffset,
                                       .y = transform.position.y};
            const engine::Vec2f aim = towardsNearestEnemy(registry, muzzle);
            commands.emplace_back(SpawnProjectile{
                .owner = entity,
                .position = muzzle,
                .velocity = aim * stats.shotSpeed,
                .damage = stats.shotDamage,
            });
        });
}

} // namespace server
