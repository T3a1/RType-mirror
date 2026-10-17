/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** DamageSystem
*/

#include "systems/DamageSystem.hpp"

#include "systems/DamageSystem.hpp"

namespace server {

namespace {

/**
 * Applies the Damage of an attacker to the Health of a target.
 * @param registry The registry holding both entities.
 * @param attacker The entity that deals the damage.
 * @param target The entity that takes it.
 * @param alreadyHit The entities that have already been hit this tick.
 * @param outbound Where the projectile destruction is announced.
 **/
void hit(engine::Registry &registry, engine::Entity attacker,
         engine::Entity target, std::vector<engine::Entity> &alreadyHit,
         Outbound &outbound)
{
    const auto *damage = registry.tryGet<rtype::Damage>(attacker);
    auto *health = registry.tryGet<rtype::Health>(target);

    if (damage == nullptr || health == nullptr || health->current <= 0 ||
        std::ranges::find(alreadyHit, attacker) != alreadyHit.end()) {
        return;
    }
    health->current -= damage->points;
    BB_DEBUG("Entity " << target.id << " hit - Health " << health->current);

    if (registry.has<rtype::Projectile>(attacker)) {
        alreadyHit.push_back(attacker);
        despawn(registry, attacker, outbound);
    }
}

} // namespace

/**
 * Applies the effects of every collision of this tick, in both directions.
 * @param registry The registry holding the entities.
 * @param collisions The pairs found by the collision system.
 * @param outbound Where the destructions are announced to the clients.
 **/
void damageSystem(engine::Registry &registry,
                  const std::vector<Collision> &collisions, Outbound &outbound)
{
    std::vector<engine::Entity> alreadyHit;

    for (const Collision &collision : collisions) {
        hit(registry, collision.a, collision.b, alreadyHit, outbound);
        hit(registry, collision.b, collision.a, alreadyHit, outbound);
    }
}

} // namespace server
