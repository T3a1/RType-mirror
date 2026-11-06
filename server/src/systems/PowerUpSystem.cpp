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
 * @brief Gives a player the boost of a power-up for its full duration.
 * @param registry The registry holding the player.
 * @param player The player's ship.
 * @param kind The boost of the power-up.
 * @param stats The weapon stats.
 **/
void applyPowerUp(engine::Registry &registry, engine::Entity player,
                  rtype::PowerUpKind kind, const WeaponStats &stats)
{
    auto *boosts = registry.tryGet<Boosts>(player);
    if (boosts == nullptr) {
        boosts = &registry.emplace<Boosts>(player);
    }
    boosts->of(kind) = stats.durationOf(kind);
    switch (kind) {
    case rtype::PowerUpKind::Spread: {
        auto *spread = registry.tryGet<SpreadShot>(player);
        if (spread == nullptr) {
            spread = &registry.emplace<SpreadShot>(player);
        }
        // The first boost goes straight to 3 shots, the next ones add 2.
        spread->level = std::min<std::uint8_t>(
            std::max<std::uint8_t>(spread->level + 1, 2), stats.maxLevel);
        BB_DEBUG("Spread level " << static_cast<int>(spread->level));
        break;
    }
    case rtype::PowerUpKind::Aimed:
        if (!registry.has<AimedShot>(player)) {
            registry.emplace<AimedShot>(player);
        }
        break;
    case rtype::PowerUpKind::Burst:
        if (!registry.has<BurstShot>(player)) {
            registry.emplace<BurstShot>(player);
        }
        break;
    case rtype::PowerUpKind::Count:
        break;
    }
}

/**
 * @brief Takes a boost away from a player whose time is up.
 * @param registry The registry holding the player.
 * @param player The player's ship.
 * @param kind The boost.
 **/
void endBoost(engine::Registry &registry, engine::Entity player,
              rtype::PowerUpKind kind)
{
    switch (kind) {
    case rtype::PowerUpKind::Spread:
        if (auto *spread = registry.tryGet<SpreadShot>(player)) {
            spread->level = 1;
        }
        break;
    case rtype::PowerUpKind::Aimed:
        registry.remove<AimedShot>(player);
        break;
    case rtype::PowerUpKind::Burst:
        registry.remove<BurstShot>(player);
        break;
    case rtype::PowerUpKind::Count:
        break;
    }
    BB_DEBUG("Boost " << rtype::powerUpName(kind) << " ended");
}

} // namespace

void powerUpDropSystem(engine::Registry &registry, SpawnCommands &commands,
                       std::mt19937 &random)
{
    std::uniform_int_distribution<std::size_t> pick(0, rtype::PowerUpKindCount -
                                                           1);
    registry.each<rtype::Enemy, rtype::Health, rtype::Transform>(
        [&](engine::Entity, const rtype::Enemy &enemy,
            const rtype::Health &health, const rtype::Transform &transform) {
            if (health.current <= 0 && enemyDef(enemy).dropsPowerUp) {
                commands.emplace_back(SpawnPowerUp{
                    .position = transform.position,
                    .kind = rtype::PowerUpKinds.at(pick(random)),
                });
            }
        });
}

void pickupSystem(engine::Registry &registry,
                  const std::vector<Collision> &collisions, Outbound &outbound,
                  const WeaponStats &stats)
{
    std::vector<engine::Entity> taken;
    const auto pick = [&](engine::Entity player, engine::Entity powerUp) {
        const auto *bonus = registry.tryGet<PowerUp>(powerUp);
        if (bonus == nullptr || !registry.has<rtype::Player>(player) ||
            std::ranges::find(taken, powerUp) != taken.end()) {
            return;
        }
        applyPowerUp(registry, player, bonus->kind, stats);
        taken.push_back(powerUp);
        despawn(registry, powerUp, outbound);
    };
    for (const Collision &collision : collisions) {
        pick(collision.a, collision.b);
        pick(collision.b, collision.a);
    }
}

void boostSystem(engine::Registry &registry, engine::Duration dt)
{
    registry.each<Boosts>([&](engine::Entity player, Boosts &boosts) {
        for (const rtype::PowerUpKind kind : rtype::PowerUpKinds) {
            engine::Duration &left = boosts.of(kind);
            if (left.count() <= 0.0F) {
                continue;
            }
            left -= dt;
            if (left.count() <= 0.0F) {
                left = engine::Duration{0.0F};
                endBoost(registry, player, kind);
            }
        }
    });
}

} // namespace server
