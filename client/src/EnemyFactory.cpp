/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** EnemyFactory
*/

#include "EnemyFactory.hpp"
#include "Window.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"

namespace client {

namespace {

constexpr int enemyLayer = 1;

} // namespace

std::optional<Sprite> enemySprite(rtype::EntityType type)
{
    switch (type) {
    case rtype::EntityType::EnemySmall:
        return Sprite{.size = {.x = 50.0F, .y = 50.0F},
                      .tint = RED,
                      .layer = enemyLayer};
    case rtype::EntityType::EnemyMedium:
        return Sprite{.size = {.x = 100.0F, .y = 50.0F},
                      .tint = BLUE,
                      .layer = enemyLayer};
    case rtype::EntityType::EnemyLarge:
        return Sprite{.size = {.x = 100.0F, .y = 100.0F},
                      .tint = GREEN,
                      .layer = enemyLayer};
    default:
        return std::nullopt;
    }
}

std::optional<engine::Entity> spawnEnemy(engine::Registry &registry, float y,
                                         engine::Vec2f velocity,
                                         rtype::EntityType type)
{
    const std::optional<Sprite> sprite = enemySprite(type);
    if (!sprite) {
        return std::nullopt;
    }

    const engine::Entity enemy = registry.create();
    registry.emplace<rtype::Transform>(
        enemy,
        rtype::Transform{
            .position = {.x = static_cast<float>(windowWidth) + sprite->size.x,
                         .y = y}});
    registry.emplace<rtype::Velocity>(enemy, rtype::Velocity{.value = velocity});
    registry.emplace<Sprite>(enemy, *sprite);
    registry.emplace<rtype::EntityType>(enemy, type);
    return enemy;
}

} // namespace client
