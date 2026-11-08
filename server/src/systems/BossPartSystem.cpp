/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BossPartSystem
*/

#include "systems/BossPartSystem.hpp"
#include "BossLayout.hpp"
#include "common/components/Transform.hpp"
#include "components/BossPart.hpp"
#include "engine/Entity.hpp"

#include <cmath>
#include <vector>

namespace server {

void bossPartSystem(engine::Registry &registry, Outbound &outbound,
                    engine::Duration dt)
{
    std::vector<engine::Entity> orphans;
    registry.each<BossPart, rtype::Transform>([&](engine::Entity entity,
                                                  BossPart &part,
                                                  rtype::Transform &transform) {
        const auto *boss = registry.alive(part.owner)
                               ? registry.tryGet<rtype::Transform>(part.owner)
                               : nullptr;
        if (boss == nullptr) {
            orphans.push_back(entity);
            return;
        }
        part.phase += BehemothTailSpeed * dt.count();
        transform.position =
            boss->position + part.offset +
            engine::Vec2f{.x = 0.0F, .y = std::sin(part.phase) * part.swing};
    });
    for (const engine::Entity entity : orphans) {
        despawn(registry, entity, outbound);
    }
}

} // namespace server
