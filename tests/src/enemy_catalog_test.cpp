/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** enemy_catalog_test
*/

#include "EnemyCatalog.hpp"
#include "EntityFactory.hpp"
#include "Level.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

namespace {

/** @brief A file adding one diving enemy drawn like a small one. */
constexpr const char *KamikazeFile = R"({ "enemies": {
    "kamikaze": { "base": "basic", "look": "small", "health": 30,
                  "speed": 380, "score": 60,
                  "pattern": { "kind": "dive", "diveDelay": 1, "diveSpeed": 900 } },
    "dreadnought": { "base": "hive", "health": 3500 }
} })";

/** @brief Parses a file, or gives the built-in catalog if it is invalid. */
server::EnemyCatalog parsed(const std::string &text)
{
    std::string error;
    return server::parseEnemies(text, error).value_or(server::EnemyCatalog{});
}

/** @brief Installs a catalog for one test, then puts the built-in one back. */
class CatalogTest : public testing::Test {
  protected:
    CatalogTest() { server::setEnemyCatalog(parsed(KamikazeFile)); }
    ~CatalogTest() override { server::setEnemyCatalog(server::EnemyCatalog{}); }
    CatalogTest(const CatalogTest &) = delete;
    CatalogTest &operator=(const CatalogTest &) = delete;
    CatalogTest(CatalogTest &&) = delete;
    CatalogTest &operator=(CatalogTest &&) = delete;

    [[nodiscard]] static server::EnemyKindId id(const char *name)
    {
        return server::enemyCatalog().find(name).value_or(0);
    }
};

} // namespace

TEST(EnemyCatalog, StartsWithTheBuiltInEnemiesByName)
{
    const server::EnemyCatalog catalog;

    ASSERT_EQ(catalog.kinds().size(), server::ENEMY_DEFS.size());
    EXPECT_EQ(catalog.find("basic"),
              server::builtinKind(rtype::EnemyType::Basic));
    EXPECT_EQ(catalog.find("warship"),
              server::builtinKind(rtype::EnemyType::Boss));
    EXPECT_EQ(catalog.find("hive"),
              server::builtinKind(rtype::EnemyType::Hive));
    EXPECT_FALSE(catalog.find("dragon").has_value());
    EXPECT_EQ(server::enemyDef(rtype::EnemyType::Large).health,
              server::ENEMY_DEFS.at(5).health);
}

TEST(EnemyCatalog, AFileChangesTheStatsOfABuiltInEnemy)
{
    const server::EnemyCatalog catalog = parsed(R"({ "enemies": {
        "basic": { "health": 40, "score": 7, "dropsPowerUp": true } } })");
    const server::EnemyDef &basic = catalog.at(0).def;

    EXPECT_EQ(catalog.kinds().size(), server::ENEMY_DEFS.size());
    EXPECT_EQ(basic.health, 40);
    EXPECT_EQ(basic.score, 7U);
    EXPECT_TRUE(basic.dropsPowerUp);
    EXPECT_FLOAT_EQ(basic.speed, server::ENEMY_DEFS.at(0).speed);
}

TEST(EnemyCatalog, ANewEnemyTakesItsBehaviorFromItsBaseAndItsSpriteFromItsLook)
{
    const server::EnemyCatalog catalog = parsed(KamikazeFile);
    const server::EnemyKind &kamikaze =
        catalog.at(catalog.find("kamikaze").value_or(0));

    EXPECT_EQ(kamikaze.name, "kamikaze");
    EXPECT_EQ(kamikaze.behavior, rtype::EnemyType::Basic);
    EXPECT_EQ(kamikaze.def.entityType, rtype::EntityType::EnemySmall);
    EXPECT_EQ(kamikaze.def.health, 30);
    EXPECT_EQ(kamikaze.def.pattern.kind, rtype::MovementKind::Dive);
    EXPECT_FLOAT_EQ(kamikaze.def.size.x, server::ENEMY_DEFS.at(0).size.x);
}

TEST(EnemyCatalog, RejectsInvalidFiles)
{
    const std::vector<std::string> invalid{
        "",
        "[]",
        R"({ "foes": {} })",
        R"({ "enemies": [] })",
        R"({ "enemies": { "basic": { "hp": 3 } } })",
        R"({ "enemies": { "basic": { "base": "wave" } } })",
        R"({ "enemies": { "basic": { "health": 0 } } })",
        R"({ "enemies": { "basic": { "health": 90000 } } })",
        R"({ "enemies": { "basic": { "health": 2.5 } } })",
        R"({ "enemies": { "basic": { "speed": -1 } } })",
        R"({ "enemies": { "basic": { "look": "dragon" } } })",
        R"({ "enemies": { "basic": { "dropsPowerUp": 1 } } })",
        R"({ "enemies": { "basic": { "pattern": { "kind": "loop" } } } })",
        R"({ "enemies": { "drone": { "health": 10 } } })",
        R"({ "enemies": { "drone": { "base": "drone" } } })",
        R"({ "enemies": { "drone": { "base": "small", "speed": "fast" } } })",
    };
    for (const std::string &text : invalid) {
        std::string error;
        EXPECT_FALSE(server::parseEnemies(text, error).has_value()) << text;
        EXPECT_FALSE(error.empty()) << text;
    }
}

TEST(EnemyCatalog, ErrorsSayWhichEnemyIsWrong)
{
    std::string error;
    EXPECT_FALSE(server::parseEnemies(
                     R"({ "enemies": { "drone": { "base": "small",
                                                  "health": -4 } } })",
                     error)
                     .has_value());
    EXPECT_NE(error.find("enemies.drone.health"), std::string::npos) << error;
}

TEST(EnemyCatalog, TheShippedFileIsValid)
{
    std::string error;
    const server::EnemyCatalog catalog =
        server::loadEnemies(RTYPE_ENEMIES_FILE, error);

    EXPECT_TRUE(error.empty()) << error;
    EXPECT_GT(catalog.kinds().size(), server::ENEMY_DEFS.size());
}

TEST(EnemyCatalog, AMissingFileGivesTheBuiltInEnemies)
{
    std::string error;
    const server::EnemyCatalog catalog =
        server::loadEnemies("/no/such/enemies.json", error);

    EXPECT_FALSE(error.empty());
    EXPECT_EQ(catalog.kinds().size(), server::ENEMY_DEFS.size());
}

TEST_F(CatalogTest, TheFactoryBuildsANewEnemyFromItsKind)
{
    engine::Registry registry;
    server::EntityFactory factory;
    const engine::Entity enemy =
        factory.create(registry, server::SpawnEnemy{.kind = id("kamikaze")});

    const auto &component = registry.get<rtype::Enemy>(enemy);
    EXPECT_EQ(component.kind, id("kamikaze"));
    EXPECT_EQ(component.type, rtype::EnemyType::Basic);
    EXPECT_EQ(registry.get<rtype::EntityType>(enemy),
              rtype::EntityType::EnemySmall);
    EXPECT_EQ(registry.get<rtype::Health>(enemy).max, 30);
    EXPECT_EQ(server::enemyDef(component).score, 60U);
}

TEST_F(CatalogTest, LevelsCallTheNewEnemiesByName)
{
    std::string error;
    const server::Level level =
        server::parseLevel(R"({ "name": "x", "boss": "dreadnought",
            "waves": [ { "time": 1, "enemy": "kamikaze", "count": 2 } ] })",
                           error)
            .value_or(server::Level{});

    ASSERT_EQ(level.waves.size(), 1U) << error;
    EXPECT_EQ(level.waves.front().formation.kind, id("kamikaze"));
    EXPECT_EQ(level.boss, rtype::EnemyType::Hive);
    EXPECT_EQ(level.bossKind, id("dreadnought"));
}

TEST_F(CatalogTest, ABossCannotComeInAWaveNorAnEnemyAsTheBoss)
{
    std::string error;
    EXPECT_FALSE(server::parseLevel(R"({ "name": "x",
        "waves": [ { "time": 1, "enemy": "dreadnought" } ] })",
                                    error)
                     .has_value());
    EXPECT_FALSE(server::parseLevel(R"({ "name": "x", "boss": "kamikaze",
        "waves": [ { "time": 1, "enemy": "basic" } ] })",
                                    error)
                     .has_value());
}
