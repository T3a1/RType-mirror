/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EntityFactory
*/

#include "EntityFactory.hpp"

namespace server {

/**
 * Creates an enemy from a spawn request.
 * @param registry The registry to create the enemy in.
 * @param request The type and position of the enemy.
 * @return The created entity.
 **/
engine::Entity EntityFactory::create(engine::Registry &registry,
                                     const SpawnEnemy &request)
{
    const EnemyDef &stat = enemyDef(request.type);
    const engine::Entity enemy = registry.create();
    const std::uint32_t networkId = m_nextNetworkId++;

    registry.emplace<rtype::NetworkId>(enemy,
                                       rtype::NetworkId{.value = networkId});
    registry.emplace<rtype::Transform>(
        enemy, rtype::Transform{.position = request.position});
    registry.emplace<rtype::Velocity>(
        enemy, rtype::Velocity{.value = {.x = -stat.speed, .y = 0.0F}});
    registry.emplace<rtype::Health>(
        enemy, rtype::Health{.current = stat.health, .max = stat.health});
    registry.emplace<rtype::Damage>(enemy);
    registry.emplace<rtype::Enemy>(enemy, rtype::Enemy{.type = request.type});
    registry.emplace<rtype::EntityType>(enemy, stat.entityType);

    registry.emplace<rtype::Collider>(
        enemy,
        rtype::Collider{.size = stat.size,
                        .layer = rtype::CollisionLayer::Enemy,
                        .mask = rtype::CollisionLayer::Player |
                                rtype::CollisionLayer::PlayerProjectile});

    BB_DEBUG("Enemy " << static_cast<int>(request.type) << " spawned with id "
                      << networkId << " at y=" << request.position.y);

    return enemy;
}

/**
 * Creates a player from a spawn request.
 * @param registry The registry to create the player in.
 * @param request The slot and position of the player.
 * @return The created entity.
 **/
engine::Entity EntityFactory::create(engine::Registry &registry,
                                     const SpawnPlayer &request)
{
    const engine::Entity player = registry.create();
    const std::uint32_t networkId = m_nextNetworkId++;

    registry.emplace<rtype::NetworkId>(player,
                                       rtype::NetworkId{.value = networkId});
    registry.emplace<rtype::Player>(player,
                                    rtype::Player{.slot = request.slot});
    registry.emplace<rtype::InputState>(player);
    registry.emplace<rtype::Transform>(
        player, rtype::Transform{.position = request.position});
    registry.emplace<rtype::Velocity>(player);
    registry.emplace<rtype::Health>(player,
                                    rtype::Health{.current = 100, .max = 100});
    registry.emplace<rtype::EntityType>(player, rtype::EntityType::PlayerShip);
    registry.emplace<rtype::Collider>(
        player, rtype::Collider{.size = {.x = 64.0F, .y = 64.0F},
                                .layer = rtype::CollisionLayer::Player,
                                .mask = rtype::CollisionLayer::Enemy |
                                        rtype::CollisionLayer::EnemyProjectile |
                                        rtype::CollisionLayer::Obstacle});

    BB_DEBUG("Player " << static_cast<int>(request.slot) << " spawned with id "
                       << networkId);
    return player;
}

} // namespace server
