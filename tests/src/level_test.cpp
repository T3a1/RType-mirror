/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** level_test
*/

#include "EntityFactory.hpp"
#include "Level.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Spawner.hpp"
#include "components/LevelProgress.hpp"
#include "components/SpawnQueue.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "systems/LevelSystem.hpp"
#include "systems/SpawnCommandSystem.hpp"
#include "systems/SpawnSystem.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace {

constexpr engine::Duration Tick{1.0F / 60.0F};

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

std::size_t countEnemies(engine::Registry &registry)
{
    std::size_t count = 0;
    registry.each<rtype::Enemy>(
        [&count](engine::Entity, const rtype::Enemy &) { ++count; });
    return count;
}

void destroyEnemies(engine::Registry &registry)
{
    registry.each<rtype::Enemy>(
        [&registry](engine::Entity enemy, const rtype::Enemy &) {
            registry.destroy(enemy);
        });
    registry.flush();
}

/** @brief A level with one enemy at the given time. */
server::Level oneEnemyAt(float time)
{
    return {.name = "test",
            .waves = {{.time = engine::Duration{time},
                       .formation = {.type = rtype::EnemyType::Basic},
                       .centerY = 500.0F}}};
}

} // namespace

TEST(Level, ReadsAWave)
{
    const auto parsed = parse(R"({
        "name": "First",
        "waves": [
            { "time": 4, "enemy": "small", "count": 3, "spacing": 0.5,
              "stepY": 20, "fromCenter": true, "y": 300,
              "pattern": { "kind": "dive", "diveDelay": 2, "diveSpeed": 600 } }
        ]
    })");

    ASSERT_TRUE(parsed.has_value());
    const server::Level level = parsed.value_or(server::Level{});
    EXPECT_EQ(level.name, "First");
    ASSERT_EQ(level.waves.size(), 1U);
    const server::Wave &wave = level.waves.front();
    EXPECT_FLOAT_EQ(wave.time.count(), 4.0F);
    EXPECT_EQ(wave.formation.type, rtype::EnemyType::Small);
    EXPECT_EQ(wave.formation.count, 3U);
    EXPECT_FLOAT_EQ(wave.formation.spacing.count(), 0.5F);
    EXPECT_TRUE(wave.formation.fromCenter);
    EXPECT_EQ(wave.centerY, 300.0F);
    ASSERT_TRUE(wave.formation.pattern.has_value());
    const rtype::MovementPattern pattern =
        wave.formation.pattern.value_or(rtype::MovementPattern{});
    EXPECT_EQ(pattern.kind, rtype::MovementKind::Dive);
    EXPECT_FLOAT_EQ(pattern.diveSpeed, 600.0F);
}

TEST(Level, DefaultsAndOrderByTime)
{
    const auto parsed = parse(R"({
        "name": "Order",
        "waves": [ { "time": 9, "enemy": "large" }, { "time": 1, "enemy": "wave" } ]
    })");

    ASSERT_TRUE(parsed.has_value());
    const server::Level level = parsed.value_or(server::Level{});
    ASSERT_EQ(level.waves.size(), 2U);
    EXPECT_EQ(level.waves.at(0).formation.type, rtype::EnemyType::Wave);
    EXPECT_EQ(level.waves.at(0).formation.count, 1U);
    EXPECT_FALSE(level.waves.at(0).centerY.has_value());
    EXPECT_FALSE(level.waves.at(0).formation.pattern.has_value());
    EXPECT_EQ(level.waves.at(1).formation.type, rtype::EnemyType::Large);
}

TEST(Level, RejectsInvalidFiles)
{
    const std::vector<std::string> invalid{
        "",
        "{ not json",
        "[]",
        R"({ "waves": [ { "time": 1, "enemy": "basic" } ] })",
        R"({ "name": "x", "waves": [] })",
        R"({ "name": "x", "waves": [ { "enemy": "basic" } ] })",
        R"({ "name": "x", "waves": [ { "time": 1, "enemy": "dragon" } ] })",
        R"({ "name": "x", "waves": [ { "time": 1, "enemy": "basic", "count": 0 } ] })",
        R"({ "name": "x", "waves": [ { "time": 1, "enemy": "basic", "count": 2.5 } ] })",
        R"({ "name": "x", "waves": [ { "time": -1, "enemy": "basic" } ] })",
        R"({ "name": "x", "waves": [ { "time": 1, "enemy": "basic", "y": 5000 } ] })",
        R"({ "name": "x", "waves": [ { "time": 1, "enemy": "basic", "speed": 3 } ] })",
        R"({ "name": "x", "waves": [ { "time": 1, "enemy": "basic", "fromCenter": 1 } ] })",
        R"({ "name": "x", "waves": [ { "time": 1, "enemy": "basic", "pattern": { "kind": "loop" } } ] })",
        R"({ "name": "x", "waves": [ { "time": "1", "enemy": "basic" } ] })",
        R"({ "name": "x", "extra": 1, "waves": [ { "time": 1, "enemy": "basic" } ] })",
        R"({ "name": "x", "boss": "dragon", "waves": [ { "time": 1, "enemy": "basic" } ] })",
    };
    for (const std::string &text : invalid) {
        std::string error;
        EXPECT_FALSE(parse(text, &error).has_value()) << text;
        EXPECT_FALSE(error.empty()) << text;
    }
}

TEST(Level, ErrorsSayWhereTheProblemIs)
{
    std::string error;
    EXPECT_FALSE(parse(R"({ "name": "x", "waves": [
        { "time": 1, "enemy": "basic" }, { "time": 2, "enemy": "basic", "count": 99 } ] })",
                       &error)
                     .has_value());
    EXPECT_NE(error.find("waves[1].count"), std::string::npos) << error;
}

TEST(Level, ShippedLevelsAreValid)
{
    const server::LevelSet set = server::loadLevels(RTYPE_LEVELS_DIR);

    EXPECT_TRUE(set.errors.empty())
        << (set.errors.empty() ? "" : set.errors.front());
    EXPECT_GE(set.levels.size(), 2U);
}

TEST(Level, MissingDirectoryIsAnErrorNotACrash)
{
    const server::LevelSet set = server::loadLevels("/no/such/levels");

    EXPECT_TRUE(set.levels.empty());
    EXPECT_FALSE(set.errors.empty());
}

/** @brief A world playing two one-enemy levels, run tick by tick. */
class LevelSystemTest : public testing::Test {
  protected:
    LevelSystemTest()
    {
        m_registry.emplace<rtype::Spawner>(
            m_world,
            rtype::Spawner{.timeUntilNextSpawn = engine::Duration{0.0F}});
        m_registry.emplace<server::SpawnQueue>(m_world);
        m_registry.emplace<server::LevelProgress>(
            m_world,
            server::LevelProgress{
                .levels = std::make_shared<const std::vector<server::Level>>(
                    std::vector{oneEnemyAt(1.0F), oneEnemyAt(0.5F)})});
    }

    void run(float seconds)
    {
        const auto ticks = static_cast<int>(seconds / Tick.count());
        for (int i = 0; i < ticks; ++i) {
            if (const auto level = server::levelSystem(m_registry, Tick)) {
                m_announced.push_back(*level);
            }
            server::spawnSystem(m_registry, m_commands, Tick);
            server::spawnCommandSystem(m_registry, m_commands, m_factory);
            m_registry.flush();
        }
    }

    engine::Registry &registry() { return m_registry; }
    [[nodiscard]] engine::Entity world() const { return m_world; }
    [[nodiscard]] const std::vector<std::uint8_t> &announced() const
    {
        return m_announced;
    }

  private:
    engine::Registry m_registry;
    server::EntityFactory m_factory;
    server::SpawnCommands m_commands;
    engine::Entity m_world = m_registry.create();
    std::vector<std::uint8_t> m_announced;
};

TEST_F(LevelSystemTest, SpawnsTheWavesOfTheLevelOnTime)
{
    run(0.9F);
    EXPECT_EQ(countEnemies(registry()), 0U);
    run(0.2F);
    EXPECT_EQ(countEnemies(registry()), 1U);
    // No random wave while a level is played.
    run(5.0F);
    EXPECT_EQ(countEnemies(registry()), 1U);
}

TEST_F(LevelSystemTest, StartsTheNextLevelOnceTheEnemiesAreGone)
{
    run(1.2F);
    destroyEnemies(registry());
    run(server::LevelPause.count() + 0.2F);
    EXPECT_EQ(countEnemies(registry()), 0U);
    EXPECT_EQ(registry().get<server::LevelProgress>(world()).level, 1U);
    run(0.5F);
    EXPECT_EQ(countEnemies(registry()), 1U);
}

TEST_F(LevelSystemTest, GoesEndlessAfterTheLastLevel)
{
    run(1.2F);
    destroyEnemies(registry());
    run(server::LevelPause.count() + 0.8F);
    destroyEnemies(registry());
    run(0.1F);
    EXPECT_FALSE(registry().has<server::LevelProgress>(world()));
    run(1.0F);
    EXPECT_GT(countEnemies(registry()), 0U);
}

TEST_F(LevelSystemTest, AnnouncesEachLevelThenTheEndlessMode)
{
    run(0.1F);
    EXPECT_EQ(announced(), (std::vector<std::uint8_t>{1}));
    run(1.1F);
    destroyEnemies(registry());
    run(0.1F);
    EXPECT_EQ(announced(), (std::vector<std::uint8_t>{1, 2}));
    run(server::LevelPause.count() + 0.8F);
    destroyEnemies(registry());
    run(0.1F);
    EXPECT_EQ(announced(), (std::vector<std::uint8_t>{1, 2, 0}));
}

TEST(Level, ReadsTheBoss)
{
    const auto parsed = parse(R"({ "name": "x", "boss": "warship",
        "waves": [ { "time": 1, "enemy": "basic" } ] })");

    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed.value_or(server::Level{}).boss, rtype::EnemyType::Boss);
}

TEST(LevelSystem, TheBossComesAfterTheWavesAndEndsTheLevel)
{
    engine::Registry registry;
    server::EntityFactory factory;
    server::SpawnCommands commands;
    server::Level level = oneEnemyAt(0.0F);
    level.boss = rtype::EnemyType::Boss;
    const engine::Entity world = registry.create();
    registry.emplace<rtype::Spawner>(world);
    registry.emplace<server::SpawnQueue>(world);
    registry.emplace<server::LevelProgress>(
        world, server::LevelProgress{
                   .levels = std::make_shared<const std::vector<server::Level>>(
                       std::vector{level, oneEnemyAt(0.0F)})});
    const auto run = [&](int ticks) {
        for (int i = 0; i < ticks; ++i) {
            static_cast<void>(server::levelSystem(registry, Tick));
            server::spawnSystem(registry, commands, Tick);
            server::spawnCommandSystem(registry, commands, factory);
            registry.flush();
        }
    };

    run(2);
    destroyEnemies(registry);
    run(2);
    ASSERT_EQ(countEnemies(registry), 1U);
    std::size_t bosses = 0;
    registry.each<rtype::Enemy>(
        [&bosses](engine::Entity, const rtype::Enemy &enemy) {
            bosses += enemy.type == rtype::EnemyType::Boss ? 1U : 0U;
        });
    EXPECT_EQ(bosses, 1U);
    EXPECT_EQ(registry.get<server::LevelProgress>(world).level, 0U);

    destroyEnemies(registry);
    run(2);
    EXPECT_EQ(registry.get<server::LevelProgress>(world).level, 1U);
}
