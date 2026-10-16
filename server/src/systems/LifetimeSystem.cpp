/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** LifetimeSystem
*/

#include "systems/LifetimeSystem.hpp"

namespace server {

/**
 * Destroys every entity whose lifetime has run out.
 * @param registry The registry holding the entities.
 * @param dt The time elapsed since the last tick.
 **/
void lifetimeSystem(engine::Registry &registry, engine::Duration dt)
{
    registry.each<rtype::Lifetime>(
        [&](engine::Entity entity, rtype::Lifetime &lifetime) {
            lifetime.remaining -= dt;
            if (lifetime.remaining.count() <= 0.0F) {
                BB_DEBUG("Lifetime expired");
                registry.destroy(entity);
            }
        });
}

} // namespace server
