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
#include "components/PowerUp.hpp"
#include "components/ShotStyles.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"

#include <algorithm>

namespace server {

namespace {

/**
 * @brief Gives a player the shot style of a power-up. Picking the style it
 * already has raises its level; a new style would replace the old one.
 * @param registry The registry holding the player.
 * @param player The player's ship.
 * @param kind The shot style of the power-up.
 * @param maxLevel The highest level of a style.
 **/
void applyPowerUp(engine::Registry &registry, engine::Entity player,
                  PowerUpKind kind, std::uint8_t maxLevel)
{
    switch (kind) {
    case PowerUpKind::Spread: {
        auto *spread = registry.tryGet<SpreadShot>(player);
        if (spread == nullptr) {
            registry.emplace<SpreadShot>(player);
            break;
        }
        spread->level = std::min<std::uint8_t>(spread->level + 1, maxLevel);
        BB_DEBUG("Spread level " << static_cast<int>(spread->level));
        break;
    }
    }
}

} // namespace

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
        const auto *bonus = registry.tryGet<PowerUp>(powerUp);
        if (bonus == nullptr || !registry.has<rtype::Player>(player) ||
            std::ranges::find(taken, powerUp) != taken.end()) {
            return;
        }
        applyPowerUp(registry, player, bonus->kind, maxLevel);
        taken.push_back(powerUp);
        despawn(registry, powerUp, outbound);
    };
    for (const Collision &collision : collisions) {
        pick(collision.a, collision.b);
        pick(collision.b, collision.a);
    }
}

} // namespace server
