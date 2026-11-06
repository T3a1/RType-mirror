/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** obstacle_test
*/

#include "EntityFactory.hpp"
#include "Level.hpp"
#include "common/Obstacles.hpp"
#include "common/Playfield.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/Player.hpp"
#include "common/components/Transform.hpp"
#include "components/LevelProgress.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/DeathSystem.hpp"
#include "systems/ObstacleSystem.hpp"
#include "systems/SpawnCommandSystem.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace {

/** @brief The time a column takes to scroll by its own width. */
constexpr float ColumnStep = rtype::TileSize.x / rtype::ObstacleSpeed;

std::optional<server::Level> parse(const std::string &text,
                                   std::string *error = nullptr)
{
    std::string message;
    auto level = server::parseLevel(text, message);
    if (error != nullptr) {
        *error = message;
    }
    return level;
}

std::size_t countType(engine::Registry &registry, rtype::EntityType wanted)
{
    std::size_t count = 0;
    registry.each<rtype::EntityType>(
        [&](engine::Entity, const rtype::EntityType &type) {
            if (type == wanted) {
                ++count;
            }
        });
    return count;
}

/** @brief Finds the collisions of the tick, then applies their damage and the
 * deaths, like the game loop does. */
void fight(engine::Registry &registry)
{
    server::Outbound outbound;
    server::damageSystem(registry, server::collisionSystem(registry), outbound);
    server::deathSystem(registry, outbound);
    registry.flush();
}

} // namespace

TEST(ObstacleLevel, SplitsAGroupIntoColumns)
{
    const auto level = parse(R"({ "name": "x",
        "waves": [ { "time": 1, "enemy": "basic" } ],
        "obstacles": [ { "time": 4, "kind": "tile", "y": 96, "rows": 3,
                         "columns": 2 } ] })");

    ASSERT_TRUE(level.has_value());
    ASSERT_EQ(level->obstacles.size(), 2U);
    for (const server::ObstacleColumn &column : level->obstacles) {
        EXPECT_TRUE(column.destroyable);
        EXPECT_FLOAT_EQ(column.top, 96.0F);
        EXPECT_EQ(column.rows, 3U);
    }
    EXPECT_FLOAT_EQ(level->obstacles.at(0).time.count(), 4.0F);
    EXPECT_FLOAT_EQ(level->obstacles.at(1).time.count(), 4.0F + ColumnStep);
}

TEST(ObstacleLevel, DefaultsToOneWallBlockAndSortsByTime)
{
    const auto level = parse(R"({ "name": "x",
        "waves": [ { "time": 1, "enemy": "basic" } ],
        "obstacles": [ { "time": 9, "kind": "wall", "y": 500 },
                       { "time": 2, "kind": "wall", "y": 100 } ] })");

    ASSERT_TRUE(level.has_value());
    ASSERT_EQ(level->obstacles.size(), 2U);
    EXPECT_FALSE(level->obstacles.at(0).destroyable);
    EXPECT_EQ(level->obstacles.at(0).rows, 1U);
    EXPECT_FLOAT_EQ(level->obstacles.at(0).top, 100.0F);
    EXPECT_FLOAT_EQ(level->obstacles.at(1).top, 500.0F);
}

TEST(ObstacleLevel, RejectsInvalidGroups)
{
    const std::string head =
        R"({ "name": "x", "waves": [ { "time": 1, "enemy": "basic" } ], )";
    const std::vector<std::string> invalid{
        R"("obstacles": {} })",
        R"("obstacles": [ { "kind": "wall", "y": 10 } ] })",
        R"("obstacles": [ { "time": 1, "y": 10 } ] })",
        R"("obstacles": [ { "time": 1, "kind": "lava", "y": 10 } ] })",
        R"("obstacles": [ { "time": 1, "kind": "wall" } ] })",
        R"("obstacles": [ { "time": 1, "kind": "wall", "y": 5000 } ] })",
        R"("obstacles": [ { "time": 1, "kind": "wall", "y": 10, "rows": 0 } ] })",
        R"("obstacles": [ { "time": 1, "kind": "wall", "y": 10, "rows": 17 } ] })",
        R"("obstacles": [ { "time": 1, "kind": "wall", "y": 10, "columns": 31 } ] })",
        R"("obstacles": [ { "time": 1, "kind": "wall", "y": 10, "rows": 1.5 } ] })",
        R"("obstacles": [ { "time": 1, "kind": "wall", "y": 10, "hp": 3 } ] })",
    };
    for (const std::string &tail : invalid) {
        std::string error;
        EXPECT_FALSE(parse(head + tail, &error).has_value()) << tail;
        EXPECT_NE(error.find("obstacles"), std::string::npos) << error;
    }
}

/** @brief A world playing one level with a column of three tiles at 1 s. */
class ObstacleSystemTest : public testing::Test {
  protected:
    ObstacleSystemTest()
    {
        server::Level level{.name = "test"};
        level.obstacles.push_back({.time = engine::Duration{1.0F},
                                   .destroyable = true,
                                   .top = 200.0F,
                                   .rows = 3});
        m_registry.emplace<server::LevelProgress>(
            m_world,
            server::LevelProgress{
                .levels = std::make_shared<const std::vector<server::Level>>(
                    std::vector{level})});
    }

    /** @brief Moves the level clock to a time and runs the system once. */
    void runAt(float seconds)
    {
        m_registry.get<server::LevelProgress>(m_world).elapsed =
            engine::Duration{seconds};
        server::obstacleSystem(m_registry, m_commands);
        server::spawnCommandSystem(m_registry, m_commands, m_factory);
    }

    engine::Registry &registry() { return m_registry; }
    server::LevelProgress &progress()
    {
        return m_registry.get<server::LevelProgress>(m_world);
    }

  private:
    engine::Registry m_registry;
    server::EntityFactory m_factory;
    server::SpawnCommands m_commands;
    engine::Entity m_world = m_registry.create();
};

TEST_F(ObstacleSystemTest, BringsInTheColumnAtItsTime)
{
    runAt(0.5F);
    EXPECT_EQ(countType(registry(), rtype::EntityType::DestroyableTile), 0U);

    runAt(1.0F);
    EXPECT_EQ(countType(registry(), rtype::EntityType::DestroyableTile), 3U);

    runAt(2.0F);
    EXPECT_EQ(countType(registry(), rtype::EntityType::DestroyableTile), 3U);
}

TEST_F(ObstacleSystemTest, StacksTheBlocksPastTheRightEdge)
{
    runAt(1.0F);

    std::vector<float> heights;
    registry().each<rtype::EntityType, rtype::Transform>(
        [&](engine::Entity, const rtype::EntityType &type,
            const rtype::Transform &transform) {
            if (type == rtype::EntityType::DestroyableTile) {
                EXPECT_GT(transform.position.x, rtype::PlayfieldWidth);
                heights.push_back(transform.position.y);
            }
        });
    EXPECT_EQ(heights, (std::vector{200.0F, 200.0F + rtype::TileSize.y,
                                    200.0F + (2.0F * rtype::TileSize.y)}));
}

TEST_F(ObstacleSystemTest, WaitsDuringThePauseBetweenLevels)
{
    progress().pause = engine::Duration{1.0F};
    runAt(5.0F);

    EXPECT_EQ(countType(registry(), rtype::EntityType::DestroyableTile), 0U);
}

/** @brief A player ship, a wall and a tile, and helpers to fire at them. */
class ObstacleCombatTest : public testing::Test {
  protected:
    ObstacleCombatTest()
        : m_ship(m_factory.create(
              m_registry,
              server::SpawnPlayer{.slot = 1,
                                  .position = {.x = 100, .y = 100}})),
          m_wall(m_factory.create(
              m_registry,
              server::SpawnObstacle{.position = {.x = 800.0F, .y = 300.0F}})),
          m_tile(m_factory.create(
              m_registry,
              server::SpawnObstacle{.position = {.x = 800.0F, .y = 700.0F},
                                    .destroyable = true}))
    {
    }

    /** @brief Creates a shot of the player over a block. */
    engine::Entity fireAt(engine::Entity block, bool charged = false)
    {
        return m_factory.create(
            m_registry,
            server::SpawnProjectile{
                .owner = m_ship,
                .position = m_registry.get<rtype::Transform>(block).position,
                .velocity = {.x = 900.0F, .y = 0.0F},
                .charged = charged,
            });
    }

    engine::Registry &registry() { return m_registry; }
    [[nodiscard]] engine::Entity ship() const { return m_ship; }
    [[nodiscard]] engine::Entity wall() const { return m_wall; }
    [[nodiscard]] engine::Entity tile() const { return m_tile; }

  private:
    engine::Registry m_registry;
    server::EntityFactory m_factory;
    engine::Entity m_ship;
    engine::Entity m_wall;
    engine::Entity m_tile;
};

TEST_F(ObstacleCombatTest, AWallStopsAShotAndStays)
{
    const engine::Entity shot = fireAt(wall());
    fight(registry());

    EXPECT_FALSE(registry().alive(shot));
    EXPECT_TRUE(registry().alive(wall()));
}

TEST_F(ObstacleCombatTest, AWallStopsEvenAChargedShot)
{
    const engine::Entity shot = fireAt(wall(), true);
    fight(registry());

    EXPECT_FALSE(registry().alive(shot));
}

TEST_F(ObstacleCombatTest, ATileBreaksAfterThreeShotsAndGivesPoints)
{
    for (int i = 0; i < 2; ++i) {
        fireAt(tile());
        fight(registry());
    }
    ASSERT_TRUE(registry().alive(tile()));
    EXPECT_LT(registry().get<rtype::Health>(tile()).current,
              rtype::TileMaxHealth);

    fireAt(tile());
    fight(registry());

    EXPECT_FALSE(registry().alive(tile()));
    EXPECT_EQ(registry().get<rtype::Player>(ship()).score, rtype::TileScore);
}

TEST_F(ObstacleCombatTest, AShipThatTouchesAWallIsDestroyed)
{
    registry().get<rtype::Transform>(ship()).position =
        registry().get<rtype::Transform>(wall()).position;
    fight(registry());

    EXPECT_FALSE(registry().alive(ship()));
    EXPECT_TRUE(registry().alive(wall()));
}
