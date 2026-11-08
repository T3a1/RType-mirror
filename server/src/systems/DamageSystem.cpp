/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** DamageSystem
*/

#include "systems/DamageSystem.hpp"

#include <algorithm>
#include <cmath>

namespace server {

namespace {

/**
 * @brief Gives the points of a destroyed enemy or tile to the player who
 * fired the projectile that destroyed it.
 * @param registry The registry holding both entities.
 * @param projectile The projectile that dealt the last hit.
 * @param target The destroyed entity.
 **/
void awardKill(engine::Registry &registry, engine::Entity projectile,
               engine::Entity target)
{
    const auto *shot = registry.tryGet<rtype::Projectile>(projectile);
    if (shot == nullptr) {
        return;
    }
    auto *shooter = registry.tryGet<rtype::Player>(shot->owner);
    if (shooter == nullptr) {
        return;
    }
    if (const auto *enemy = registry.tryGet<rtype::Enemy>(target)) {
        shooter->score += enemyDef(*enemy).score;
    } else if (registry.has<Obstacle>(target)) {
        shooter->score += rtype::TileScore;
    }
}

/**
 * @brief Applies the Damage of an attacker to the Health of a target.
 * @param registry The registry holding both entities.
 * @param attacker The entity that deals the damage.
 * @param target The entity that takes it.
 * @param alreadyHit The entities that have already been hit this tick.
 * @param outbound Where the projectile destruction is announced.
 * @param invinciblePlayers Whether a player's ship keeps its health.
 **/
void hit(engine::Registry &registry, engine::Entity attacker,
         engine::Entity target, std::vector<engine::Entity> &alreadyHit,
         Outbound &outbound, bool invinciblePlayers)
{
    // A part of a boss passes the damage on to its boss.
    float factor = 1.0F;
    if (const auto *part = registry.tryGet<BossPart>(target)) {
        if (!registry.alive(part->owner)) {
            return;
        }
        factor = part->damageFactor;
        target = part->owner;
    }
    const auto *damage = registry.tryGet<rtype::Damage>(attacker);
    auto *health = registry.tryGet<rtype::Health>(target);
    auto *piercing = registry.tryGet<Piercing>(attacker);
    const bool isShot = registry.has<rtype::Projectile>(attacker);
    // The level stops every shot, piercing or not.
    const bool blocked = isShot && registry.has<Obstacle>(target);

    if (damage == nullptr ||
        std::ranges::find(alreadyHit, attacker) != alreadyHit.end()) {
        return;
    }
    if (health == nullptr || health->current <= 0) {
        if (blocked) {
            alreadyHit.push_back(attacker);
            despawn(registry, attacker, outbound);
        }
        return;
    }
    // A piercing shot damages each entity once and goes on.
    if (piercing != nullptr && !blocked) {
        if (std::ranges::find(piercing->hit, target) != piercing->hit.end()) {
            return;
        }
        piercing->hit.push_back(target);
    }
    if (!invinciblePlayers || !registry.has<rtype::Player>(target)) {
        health->current -=
            std::max(1, static_cast<int>(std::lround(
                            static_cast<float>(damage->points) * factor)));
    }
    BB_DEBUG("Entity " << target.id << " hit - Health " << health->current);
    if (health->current <= 0) {
        awardKill(registry, attacker, target);
    }

    if (isShot && (piercing == nullptr || blocked)) {
        alreadyHit.push_back(attacker);
        despawn(registry, attacker, outbound);
    }
}

/**
 * @brief Tells whether an entity is a weak point of a boss.
 * @param registry The registry holding it.
 * @param entity The entity.
 * @return `true` for a BossPart that takes more than the damage of a shot.
 **/
bool weakPoint(engine::Registry &registry, engine::Entity entity)
{
    const auto *part = registry.tryGet<BossPart>(entity);
    return part != nullptr && part->damageFactor > 1.0F;
}

} // namespace

void damageSystem(engine::Registry &registry,
                  const std::vector<Collision> &collisions, Outbound &outbound,
                  bool invinciblePlayers)
{
    std::vector<engine::Entity> alreadyHit;
    // A shot that touches a weak point and the body around it hits the weak
    // point.
    std::vector<Collision> ordered = collisions;
    std::ranges::stable_partition(ordered, [&](const Collision &collision) {
        return weakPoint(registry, collision.a) ||
               weakPoint(registry, collision.b);
    });

    for (const Collision &collision : ordered) {
        hit(registry, collision.a, collision.b, alreadyHit, outbound,
            invinciblePlayers);
        hit(registry, collision.b, collision.a, alreadyHit, outbound,
            invinciblePlayers);
    }
}

} // namespace server
