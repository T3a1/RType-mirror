/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EntityFactory
*/

#include "EntityFactory.hpp"
#include "BossLayout.hpp"
#include "common/Obstacles.hpp"
#include "common/Playfield.hpp"
#include "components/ShotStyles.hpp"

#include <algorithm>
#include <cmath>

namespace server {

namespace {

/** @brief The collider of a normal shot, in playfield units. */
constexpr engine::Vec2f ShotSize{.x = 16.0F, .y = 8.0F};
/** @brief The collider of a charged shot, in playfield units. */
constexpr engine::Vec2f ChargedShotSize{.x = 200.0F, .y = 40.0F};
/** @brief The collider of a power-up, in playfield units. */
constexpr engine::Vec2f PowerUpSize{.x = 51.0F, .y = 54.0F};
/** @brief How fast a power-up flies to the left, in playfield units per second.
 */
constexpr float PowerUpSpeed = 120.0F;

} // namespace

static_assert(rtype::Damage{}.points >= rtype::PlayerMaxHealth,
              "a single collision with an enemy must kill a player");

engine::Entity EntityFactory::create(engine::Registry &registry,
                                     const SpawnEnemy &request)
{
    const EnemyKindId id = request.kind.value_or(builtinKind(request.type));
    const EnemyKind &kind = enemyCatalog().at(id);
    const EnemyDef &stat = kind.def;
    const rtype::EnemyType type = kind.behavior;
    const engine::Entity enemy = registry.create();
    const std::uint32_t networkId = m_nextNetworkId++;

    registry.emplace<rtype::NetworkId>(enemy,
                                       rtype::NetworkId{.value = networkId});
    registry.emplace<rtype::Transform>(
        enemy, rtype::Transform{.position = request.position});
    registry.emplace<rtype::Velocity>(
        enemy, rtype::Velocity{.value = {.x = -stat.speed, .y = 0.0F}});
    const auto health = std::max(
        1, static_cast<int>(std::lround(static_cast<float>(stat.health) *
                                        m_difficulty.enemyHealth)));
    const bool boss = rtype::isBoss(type);
    if (boss) {
        // bossSystem() moves it, and it never leaves the playfield.
        registry.emplace<Boss>(enemy);
    } else {
        registry.emplace<rtype::MovementPattern>(
            enemy, request.pattern.value_or(stat.pattern));
    }
    registry.emplace<rtype::Health>(
        enemy, rtype::Health{.current = health, .max = health});
    registry.emplace<rtype::Damage>(enemy);
    registry.emplace<rtype::Enemy>(enemy,
                                   rtype::Enemy{.type = type, .kind = id});
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
            enemy,
            rtype::Weapon{.fireInterval = stat.fireInterval *
                                          m_difficulty.enemyFireInterval});
    }

    if (boss) {
        createBossParts(registry, enemy, type, stat);
    }

    BB_DEBUG("Enemy " << kind.name << " spawned with id " << networkId
                      << " at y=" << request.position.y);

    return enemy;
}

void EntityFactory::createBossParts(engine::Registry &registry,
                                    engine::Entity boss, rtype::EnemyType type,
                                    const EnemyDef &stat)
{
    if (type == rtype::EnemyType::Twin) {
        // The boss stands on the ground, this half hangs from the ceiling.
        createBossPart(
            registry,
            {.owner = boss,
             .offset = {.x = 0.0F,
                        .y = (TwinTopSize.y / 2.0F) -
                             (rtype::PlayfieldHeight - (stat.size.y / 2.0F))}},
            rtype::EntityType::BossTwinTop, TwinTopSize);
        return;
    }
    if (type != rtype::EnemyType::Behemoth) {
        return;
    }
    createBossPart(registry,
                   {.owner = boss,
                    .offset = BehemothHeadOffset,
                    .damageFactor = BehemothHeadDamage},
                   rtype::EntityType::BehemothHead, BehemothHeadSize);
    for (std::size_t i = 0; i < BehemothTailSegments; ++i) {
        const auto index = static_cast<float>(i);
        const bool tip = i + 1 == BehemothTailSegments;
        createBossPart(
            registry,
            {.owner = boss,
             .offset = {.x = BehemothTailBase.x - (index * BehemothTailSpacing),
                        .y = BehemothTailBase.y},
             .damageFactor = BehemothTailDamage,
             .swing = (index + 1.0F) * BehemothTailSwing,
             .phase = -index * 0.5F},
            tip ? rtype::EntityType::BehemothTailTip
                : rtype::EntityType::BehemothTail,
            tip ? BehemothTailTipSize : BehemothTailSize);
    }
}

engine::Entity EntityFactory::createBossPart(engine::Registry &registry,
                                             const BossPart &part,
                                             rtype::EntityType look,
                                             engine::Vec2f size)
{
    const engine::Entity entity = registry.create();
    const auto &boss = registry.get<rtype::Transform>(part.owner);
    registry.emplace<rtype::NetworkId>(
        entity, rtype::NetworkId{.value = m_nextNetworkId++});
    registry.emplace<rtype::Transform>(
        entity, rtype::Transform{.position = boss.position + part.offset});
    registry.emplace<rtype::EntityType>(entity, look);
    registry.emplace<rtype::Damage>(entity);
    registry.emplace<rtype::Collider>(
        entity, rtype::Collider{
                    .size = size,
                    .layer = rtype::CollisionLayer::Enemy,
                    .mask = rtype::CollisionLayer::Player |
                            rtype::CollisionLayer::PlayerProjectile,
                });
    registry.emplace<BossPart>(entity, part);
    return entity;
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
                            rtype::CollisionLayer::Obstacle |
                            rtype::CollisionLayer::Pickup,
                });

    registry.emplace<KeepInPlayfield>(player);
    registry.emplace<rtype::Weapon>(player);
    registry.emplace<SpreadShot>(player);
    registry.emplace<Boosts>(player);

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
    registry.emplace<rtype::Damage>(projectile,
                                    rtype::Damage{.points = request.damage});

    if (request.fromPlayer) {
        registry.emplace<rtype::EntityType>(
            projectile, request.charged ? rtype::EntityType::ChargedShot
                                        : rtype::EntityType::PlayerMissile);
        if (request.charged) {
            registry.emplace<Piercing>(projectile);
        }
        registry.emplace<rtype::Collider>(
            projectile,
            rtype::Collider{
                .size = request.charged ? ChargedShotSize : ShotSize,
                .layer = rtype::CollisionLayer::PlayerProjectile,
                .mask = rtype::CollisionLayer::Enemy |
                        rtype::CollisionLayer::Obstacle,
            });
    } else {
        registry.emplace<rtype::EntityType>(projectile, request.look);
        registry.emplace<rtype::Collider>(
            projectile, rtype::Collider{
                            .size = ShotSize,
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

engine::Entity EntityFactory::create(engine::Registry &registry,
                                     const SpawnPowerUp &request)
{
    const engine::Entity powerUp = registry.create();
    const std::uint32_t networkId = m_nextNetworkId++;

    registry.emplace<rtype::NetworkId>(powerUp,
                                       rtype::NetworkId{.value = networkId});
    registry.emplace<rtype::Transform>(
        powerUp, rtype::Transform{.position = request.position});
    registry.emplace<rtype::Velocity>(
        powerUp, rtype::Velocity{.value = {.x = -PowerUpSpeed, .y = 0.0F}});
    registry.emplace<rtype::EntityType>(powerUp, rtype::EntityType::PowerUp);
    registry.emplace<rtype::Collider>(
        powerUp, rtype::Collider{
                     .size = PowerUpSize,
                     .layer = rtype::CollisionLayer::Pickup,
                     .mask = rtype::CollisionLayer::Player,
                 });
    registry.emplace<PowerUp>(powerUp, PowerUp{.kind = request.kind});
    registry.emplace<DestroyOffScreen>(powerUp);

    BB_DEBUG("Power-up spawned with id " << networkId);
    return powerUp;
}

engine::Entity EntityFactory::create(engine::Registry &registry,
                                     const SpawnObstacle &request)
{
    const engine::Entity block = registry.create();
    const std::uint32_t networkId = m_nextNetworkId++;

    registry.emplace<rtype::NetworkId>(block,
                                       rtype::NetworkId{.value = networkId});
    registry.emplace<rtype::Transform>(
        block, rtype::Transform{.position = request.position});
    registry.emplace<rtype::Velocity>(
        block,
        rtype::Velocity{.value = {.x = -rtype::ObstacleSpeed, .y = 0.0F}});
    registry.emplace<rtype::EntityType>(
        block, request.destroyable ? rtype::EntityType::DestroyableTile
                                   : rtype::EntityType::Obstacle);
    registry.emplace<rtype::Collider>(
        block, rtype::Collider{
                   .size = rtype::TileSize,
                   .layer = rtype::CollisionLayer::Obstacle,
                   .mask = rtype::CollisionLayer::Player |
                           rtype::CollisionLayer::PlayerProjectile |
                           rtype::CollisionLayer::EnemyProjectile,
               });
    // A ship that touches a block is destroyed, like on an enemy.
    registry.emplace<rtype::Damage>(block);
    registry.emplace<Obstacle>(block,
                               Obstacle{.destroyable = request.destroyable});
    if (request.destroyable) {
        registry.emplace<rtype::Health>(block,
                                        rtype::Health{
                                            .current = rtype::TileMaxHealth,
                                            .max = rtype::TileMaxHealth,
                                        });
    }
    registry.emplace<DestroyOffScreen>(block);

    BB_DEBUG((request.destroyable ? "Tile" : "Wall")
             << " spawned with id " << networkId);
    return block;
}

} // namespace server
