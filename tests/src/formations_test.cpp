/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** formations_test
*/

#include "EnemyDefs.hpp"
#include "EntityFactory.hpp"
#include "Formations.hpp"
#include "common/Playfield.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/MovementPattern.hpp"
#include "common/components/Spawner.hpp"
#include "common/components/Transform.hpp"
#include "common/systems/MovementSystem.hpp"
#include "common/systems/PatternSystem.hpp"
#include "components/SpawnQueue.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "systems/SpawnSystem.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

namespace {

constexpr engine::Duration Tick{1.0F / 60.0F};
// BoundarySystem destroys what is further than this past the playfield.
constexpr float DespawnMargin = 200.0F;

std::size_t countEnemies(engine::Registry &registry)
{
    std::size_t count = 0;
    registry.each<rtype::Enemy>(
        [&count](engine::Entity, const rtype::Enemy &) { ++count; });
    return count;
}

} // namespace

TEST(EnemyDefs, TypesAreInTheOrderOfEnemyType)
{
    EXPECT_EQ(server::enemyDef(rtype::EnemyType::Basic).entityType,
              rtype::EntityType::EnemyBasic);
    EXPECT_EQ(server::enemyDef(rtype::EnemyType::Wave).entityType,
              rtype::EntityType::EnemyWave);
    EXPECT_EQ(server::enemyDef(rtype::EnemyType::Shooter).entityType,
              rtype::EntityType::EnemyShooter);
    EXPECT_EQ(server::enemyDef(rtype::EnemyType::Small).entityType,
              rtype::EntityType::EnemySmall);
    EXPECT_EQ(server::enemyDef(rtype::EnemyType::Medium).entityType,
              rtype::EntityType::EnemyMedium);
    EXPECT_EQ(server::enemyDef(rtype::EnemyType::Large).entityType,
              rtype::EntityType::EnemyLarge);
}

TEST(Formations, EveryFormationFitsInThePlayfield)
{
    for (const server::Formation &formation : server::FORMATIONS) {
        const float extent = server::formationExtent(formation);
        ASSERT_LE(extent * 2.0F, rtype::PlayfieldHeight);

        const auto spawns = server::formationSpawns(formation, extent);
        const float margin = extent - (static_cast<float>(formation.count - 1) /
                                       2.0F * formation.stepY);
        EXPECT_GE(spawns.front().request.position.y, margin - 1e-3F);
    }
}

TEST(Formations, EnemiesSpawnBetweenTheEdgeAndTheDespawnLine)
{
    for (const server::Formation &formation : server::FORMATIONS) {
        for (const server::PendingSpawn &spawn :
             server::formationSpawns(formation, 500.0F)) {
            EXPECT_GT(spawn.request.position.x, rtype::PlayfieldWidth);
            EXPECT_LT(spawn.request.position.x,
                      rtype::PlayfieldWidth + DespawnMargin);
        }
    }
}

TEST(Formations, SnakeSpawnsOneAfterTheOtherAtTheSameHeight)
{
    const server::Formation snake{.type = rtype::EnemyType::Small,
                                  .count = 3,
                                  .spacing = engine::Duration{0.5F}};

    const auto spawns = server::formationSpawns(snake, 400.0F);

    ASSERT_EQ(spawns.size(), 3U);
    EXPECT_FLOAT_EQ(spawns.at(0).delay.count(), 0.0F);
    EXPECT_FLOAT_EQ(spawns.at(1).delay.count(), 0.5F);
    EXPECT_FLOAT_EQ(spawns.at(2).delay.count(), 1.0F);
    for (const server::PendingSpawn &spawn : spawns) {
        EXPECT_FLOAT_EQ(spawn.request.position.y, 400.0F);
    }
}

TEST(Formations, VStartsFromItsMiddle)
{
    const server::Formation vee{.type = rtype::EnemyType::Basic,
                                .count = 3,
                                .spacing = engine::Duration{0.5F},
                                .stepY = 100.0F,
                                .fromCenter = true};

    const auto spawns = server::formationSpawns(vee, 400.0F);

    ASSERT_EQ(spawns.size(), 3U);
    EXPECT_FLOAT_EQ(spawns.at(0).delay.count(), 0.5F);
    EXPECT_FLOAT_EQ(spawns.at(1).delay.count(), 0.0F);
    EXPECT_FLOAT_EQ(spawns.at(2).delay.count(), 0.5F);
    EXPECT_FLOAT_EQ(spawns.at(0).request.position.y, 300.0F);
    EXPECT_FLOAT_EQ(spawns.at(1).request.position.y, 400.0F);
    EXPECT_FLOAT_EQ(spawns.at(2).request.position.y, 500.0F);
}

TEST(EntityFactory, EnemyFollowsThePathOfItsType)
{
    engine::Registry registry;
    server::EntityFactory factory;

    const engine::Entity enemy = factory.create(
        registry, server::SpawnEnemy{.type = rtype::EnemyType::Medium});

    EXPECT_EQ(registry.get<rtype::MovementPattern>(enemy).kind,
              rtype::MovementKind::Zigzag);
}

TEST(EntityFactory, RequestOverridesThePath)
{
    engine::Registry registry;
    server::EntityFactory factory;

    const engine::Entity enemy = factory.create(
        registry,
        server::SpawnEnemy{
            .type = rtype::EnemyType::Medium,
            .pattern =
                rtype::MovementPattern{.kind = rtype::MovementKind::Dive},
        });

    EXPECT_EQ(registry.get<rtype::MovementPattern>(enemy).kind,
              rtype::MovementKind::Dive);
}

TEST(SpawnSystem, SpawnsQueuedEnemiesWhenTheyAreDue)
{
    engine::Registry registry;
    server::EntityFactory factory;
    const engine::Entity world = registry.create();
    registry.emplace<rtype::Spawner>(
        world, rtype::Spawner{.timeUntilNextSpawn = engine::Duration{1000.0F}});
    registry.emplace<server::SpawnQueue>(
        world, server::SpawnQueue{.pending = {
                                      {.delay = engine::Duration{0.0F}},
                                      {.delay = engine::Duration{0.5F}},
                                  }});

    server::spawnSystem(registry, factory, Tick);
    EXPECT_EQ(countEnemies(registry), 1U);
    EXPECT_EQ(registry.get<server::SpawnQueue>(world).pending.size(), 1U);

    for (int i = 0; i < 31; ++i) {
        server::spawnSystem(registry, factory, Tick);
    }
    EXPECT_EQ(countEnemies(registry), 2U);
    EXPECT_TRUE(registry.get<server::SpawnQueue>(world).pending.empty());
}

TEST(SpawnSystem, FormationsKeepComingAndStayInThePlayfield)
{
    engine::Registry registry;
    server::EntityFactory factory;
    const engine::Entity world = registry.create();
    registry.emplace<rtype::Spawner>(world);
    registry.emplace<server::SpawnQueue>(world);

    for (int i = 0; i < 60 * 60; ++i) {
        server::spawnSystem(registry, factory, Tick);
        rtype::patternSystem(registry, Tick);
        rtype::movementSystem(registry, Tick);
        registry.each<rtype::Enemy, rtype::Transform>(
            [](engine::Entity, const rtype::Enemy &,
               const rtype::Transform &transform) {
                EXPECT_GE(transform.position.y, 0.0F);
                EXPECT_LE(transform.position.y, rtype::PlayfieldHeight);
            });
    }

    EXPECT_GT(countEnemies(registry), 10U);
}
