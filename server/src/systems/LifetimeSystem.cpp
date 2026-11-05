/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** LifetimeSystem
*/

#include "systems/LifetimeSystem.hpp"

namespace server {

void lifetimeSystem(engine::Registry &registry, engine::Duration dt,
                    Outbound &outbound)
{
    registry.each<rtype::Lifetime>(
        [&](engine::Entity entity, rtype::Lifetime &lifetime) {
            lifetime.remaining -= dt;
            if (lifetime.remaining.count() <= 0.0F) {
                BB_DEBUG("Lifetime expired");
                despawn(registry, entity, outbound);
            }
        });
}

} // namespace server
