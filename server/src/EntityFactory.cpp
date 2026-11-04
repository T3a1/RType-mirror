/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EntityFactory
*/

#include "EntityFactory.hpp"
#include "common/Playfield.hpp"

namespace server {

static_assert(rtype::Damage{}.points >= rtype::PlayerMaxHealth,
              "a single collision with an enemy must kill a player");

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
    const bool boss = request.type == rtype::EnemyType::Boss;
    if (boss) {
        // bossSystem() moves it, and it never leaves the playfield.
        registry.emplace<Boss>(enemy);
    } else {
        registry.emplace<rtype::MovementPattern>(
            enemy, request.pattern.value_or(stat.pattern));
    }
    registry.emplace<rtype::Health>(
        enemy, rtype::Health{.current = stat.health, .max = stat.health});
    registry.emplace<rtype::Damage>(enemy);
    registry.emplace<rtype::Enemy>(enemy, rtype::Enemy{.type = request.type});
    registry.emplace<rtype::EntityType>(enemy, stat.entityType);

    registry.emplace<rtype::Collider>(
        enemy, rtype::Collider{
                   .size = stat.size,
                   .layer = rtype::CollisionLayer::Enemy,
                   .mask = rtype::CollisionLayer::Player |
                           rtype::CollisionLayer::PlayerProjectile,
               });

    if (!boss) {
        registry.emplace<DestroyOffScreen>(enemy);
    }
    if (stat.fireInterval.count() > 0.0F) {
        registry.emplace<rtype::Weapon>(
            enemy, rtype::Weapon{.fireInterval = stat.fireInterval});
    }

    BB_DEBUG("Enemy " << static_cast<int>(request.type) << " spawned with id "
                      << networkId << " at y=" << request.position.y);

    return enemy;
}

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
                                    rtype::Health{
                                        .current = rtype::PlayerMaxHealth,
                                        .max = rtype::PlayerMaxHealth,
                                    });
    registry.emplace<rtype::EntityType>(player, rtype::EntityType::PlayerShip);
    registry.emplace<rtype::Collider>(
        player, rtype::Collider{
                    .size = rtype::PlayerShipSize,
                    .layer = rtype::CollisionLayer::Player,
                    .mask = rtype::CollisionLayer::Enemy |
                            rtype::CollisionLayer::EnemyProjectile |
                            rtype::CollisionLayer::Obstacle,
                });

    registry.emplace<KeepInPlayfield>(player);
    registry.emplace<rtype::Weapon>(player);

    BB_DEBUG("Player " << static_cast<int>(request.slot) << " spawned with id "
                       << networkId);
    return player;
}

engine::Entity EntityFactory::create(engine::Registry &registry,
                                     const SpawnProjectile &request)
{
    const engine::Entity projectile = registry.create();
    const std::uint32_t networkId = m_nextNetworkId++;

    registry.emplace<rtype::NetworkId>(projectile,
                                       rtype::NetworkId{.value = networkId});
    registry.emplace<rtype::Transform>(
        projectile, rtype::Transform{.position = request.position});
    registry.emplace<rtype::Velocity>(
        projectile, rtype::Velocity{.value = request.velocity});
    registry.emplace<rtype::Projectile>(
        projectile, rtype::Projectile{.owner = request.owner});
    registry.emplace<rtype::Damage>(projectile, rtype::Damage{.points = 50});

    if (request.fromPlayer) {
        registry.emplace<rtype::EntityType>(projectile,
                                            rtype::EntityType::PlayerMissile);
        registry.emplace<rtype::Collider>(
            projectile, rtype::Collider{
                            .size = {.x = 16.0F, .y = 8.0F},
                            .layer = rtype::CollisionLayer::PlayerProjectile,
                            .mask = rtype::CollisionLayer::Enemy |
                                    rtype::CollisionLayer::Obstacle,
                        });
    } else {
        registry.emplace<rtype::EntityType>(projectile,
                                            rtype::EntityType::EnemyMissile);
        registry.emplace<rtype::Collider>(
            projectile, rtype::Collider{
                            .size = {.x = 16.0F, .y = 8.0F},
                            .layer = rtype::CollisionLayer::EnemyProjectile,
                            .mask = rtype::CollisionLayer::Player |
                                    rtype::CollisionLayer::Obstacle,
                        });
    }

    // Long enough to cross the whole playfield, even at the slowest speed.
    registry.emplace<rtype::Lifetime>(
        projectile, rtype::Lifetime{.remaining = engine::Duration{5.0F}});
    registry.emplace<DestroyOffScreen>(projectile);

    BB_DEBUG("Projectile spawned with id " << networkId
                                           << " at x=" << request.position.x);
    return projectile;
}

} // namespace server
