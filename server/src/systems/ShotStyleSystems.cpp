/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ShotStyleSystems
*/

#include "systems/ShotStyleSystems.hpp"
#include "SpawnRequests.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Weapon.hpp"
#include "components/ShotStyles.hpp"
#include "engine/Entity.hpp"

#include <cmath>
#include <vector>

namespace server {

void spreadShotSystem(engine::Registry &registry, EntityFactory &factory,
                      const WeaponStats &stats)
{
    std::vector<SpawnProjectile> shots;
    registry.each<rtype::Weapon, SpreadShot, rtype::Transform>(
        [&](engine::Entity entity, const rtype::Weapon &weapon,
            const SpreadShot &spread, const rtype::Transform &transform) {
            if (!weapon.fired) {
                return;
            }
            const int side = static_cast<int>(spread.level) - 1;
            for (int i = -side; i <= side; ++i) {
                const float angle = static_cast<float>(i) * stats.spreadAngle;
                shots.push_back({
                    .owner = entity,
                    .position = {.x = transform.position.x + MuzzleOffset,
                                 .y = transform.position.y},
                    .velocity = {.x = std::cos(angle) * stats.shotSpeed,
                                 .y = std::sin(angle) * stats.shotSpeed},
                    .damage = stats.shotDamage,
                });
            }
        });
    for (const SpawnProjectile &shot : shots) {
        factory.create(registry, shot);
    }
}

} // namespace server
