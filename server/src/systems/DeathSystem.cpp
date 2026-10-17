/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** DeathSystem
*/

#include "systems/DeathSystem.hpp"

namespace server {

/**
 * Removes every entity whose health has run out.
 * @param registry The registry holding the entities.
 * @param outbound Where the deaths are announced to the clients.
 **/
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
