/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** PowerUpSystem
*/

#include "systems/PowerUpSystem.hpp"
#include "Despawn.hpp"
#include "EnemyDefs.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Health.hpp"
#include "common/components/Player.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Weapon.hpp"
#include "components/PowerUp.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"

#include <algorithm>

namespace server {

void powerUpDropSystem(engine::Registry &registry, EntityFactory &factory)
{
    std::vector<SpawnPowerUp> drops;
    registry.each<rtype::Enemy, rtype::Health, rtype::Transform>(
        [&drops](engine::Entity, const rtype::Enemy &enemy,
                 const rtype::Health &health,
                 const rtype::Transform &transform) {
            if (health.current <= 0 && enemyDef(enemy.type).dropsPowerUp) {
                drops.push_back({.position = transform.position});
            }
        });
    for (const SpawnPowerUp &drop : drops) {
        factory.create(registry, drop);
    }
}

void pickupSystem(engine::Registry &registry,
                  const std::vector<Collision> &collisions, Outbound &outbound,
                  std::uint8_t maxLevel)
{
    std::vector<engine::Entity> taken;
    const auto pick = [&](engine::Entity player, engine::Entity powerUp) {
        auto *weapon = registry.tryGet<rtype::Weapon>(player);
        if (weapon == nullptr || !registry.has<rtype::Player>(player) ||
            !registry.has<PowerUp>(powerUp) ||
            std::ranges::find(taken, powerUp) != taken.end()) {
            return;
        }
        weapon->level = std::min<std::uint8_t>(weapon->level + 1, maxLevel);
        BB_DEBUG("Weapon level " << static_cast<int>(weapon->level));
        taken.push_back(powerUp);
        despawn(registry, powerUp, outbound);
    };
    for (const Collision &collision : collisions) {
        pick(collision.a, collision.b);
        pick(collision.b, collision.a);
    }
}

} // namespace server
