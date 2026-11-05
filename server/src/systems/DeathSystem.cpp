/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** DeathSystem
*/

#include "systems/DeathSystem.hpp"

namespace server {

void deathSystem(engine::Registry &registry, Outbound &outbound)
{
    registry.each<rtype::Health>(
        [&](engine::Entity entity, const rtype::Health &health) {
            if (health.current <= 0) {
                BB_DEBUG("Entity " << entity.id << " died");
                despawn(registry, entity, outbound);
            }
        });
}

} // namespace server
