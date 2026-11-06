/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** weapons_test
*/

#include "EntityFactory.hpp"
#include "Weapons.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/PowerUpKind.hpp"
#include "common/components/Projectile.hpp"
#include "common/components/Velocity.hpp"
#include "common/components/Weapon.hpp"
#include "components/Piercing.hpp"
#include "components/PowerUp.hpp"
#include "components/ShotStyles.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/PowerUpSystem.hpp"
#include "systems/ShootSystem.hpp"
#include "systems/ShotStyleSystems.hpp"
#include "systems/SpawnCommandSystem.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <random>
#include <set>
#include <string>
#include <vector>

namespace {

constexpr engine::Duration Tick{1.0F / 60.0F};

std::size_t countType(engine::Registry &registry, rtype::EntityType wanted)
{
    std::size_t count = 0;
    registry.each<rtype::EntityType>(
        [&](engine::Entity, const rtype::EntityType &type) {
            count += type == wanted ? 1U : 0U;
        });
    return count;
}

/** @brief One player ship, shooting with the default weapon stats. */
class WeaponTest : public testing::Test {
  protected:
    WeaponTest()
        : m_ship(m_factory.create(
              m_registry,
              server::SpawnPlayer{.slot = 1,
                                  .position = {.x = 200.0F, .y = 500.0F}}))
    {
    }

    /** @brief Holds the fire key (or not) for some ticks. */
    void hold(bool fire, int ticks)
    {
        for (int i = 0; i < ticks; ++i) {
            m_registry.get<rtype::Weapon>(m_ship).triggered = fire;
            server::shootSystem(m_registry, m_commands, Tick, m_stats);
            server::burstShotSystem(m_registry, Tick, m_stats);
            server::spreadShotSystem(m_registry, m_commands, m_stats);
            server::aimedShotSystem(m_registry, m_commands, m_stats);
            server::spawnCommandSystem(m_registry, m_commands, m_factory);
            m_registry.flush();
        }
    }

    /** @brief Waits long enough for the weapon to fire again. */
    void cool()
    {
        hold(false, static_cast<int>(m_stats.fireInterval / Tick) + 2);
    }

    /** @brief Makes the ship touch a power-up of a kind. */
    void pick(rtype::PowerUpKind kind)
    {
        const engine::Entity powerUp =
            m_factory.create(m_registry, server::SpawnPowerUp{.kind = kind});
        server::Outbound outbound;
        server::pickupSystem(m_registry, {{.a = m_ship, .b = powerUp}},
                             outbound, m_stats);
        m_registry.flush();
    }

    /** @brief Lets the boosts run for some seconds. */
    void wait(float seconds)
    {
        server::boostSystem(m_registry, engine::Duration{seconds});
    }

    /** @brief The time left of a boost of the ship. */
    float left(rtype::PowerUpKind kind)
    {
        return m_registry.get<server::Boosts>(m_ship).of(kind).count();
    }

    engine::Registry &registry() { return m_registry; }
    [[nodiscard]] engine::Entity ship() const { return m_ship; }
    [[nodiscard]] const server::WeaponStats &stats() const { return m_stats; }

  private:
    engine::Registry m_registry;
    server::EntityFactory m_factory;
    server::SpawnCommands m_commands;
    server::WeaponStats m_stats;
    engine::Entity m_ship;
};

} // namespace

TEST(Weapons, ReadsTheFileAndKeepsDefaultsForMissingKeys)
{
    std::string error;
    const auto stats =
        server::parseWeapons(R"({ "shotDamage": 80, "maxLevel": 4 })", error);

    ASSERT_TRUE(stats.has_value()) << error;
    const server::WeaponStats read = stats.value_or(server::WeaponStats{});
    EXPECT_EQ(read.shotDamage, 80);
    EXPECT_EQ(read.maxLevel, 4);
    EXPECT_FLOAT_EQ(read.shotSpeed, server::WeaponStats{}.shotSpeed);
}

TEST(Weapons, RejectsInvalidFiles)
{
    for (const std::string text :
         {"", "[]", R"({ "laser": 1 })", R"({ "shotDamage": 2.5 })",
          R"({ "maxLevel": 9 })", R"({ "chargeTime": "long" })"}) {
        std::string error;
        EXPECT_FALSE(server::parseWeapons(text, error).has_value()) << text;
        EXPECT_FALSE(error.empty()) << text;
    }
}

TEST(Weapons, ShippedFileIsValid)
{
    std::string error;
    static_cast<void>(server::loadWeapons(RTYPE_WEAPONS_FILE, error));
    EXPECT_TRUE(error.empty()) << error;
}

TEST_F(WeaponTest, EachPressFiresAndEachLevelAddsTwoShots)
{
    hold(true, 1);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile), 1U);

    registry().get<server::SpreadShot>(ship()).level = 3;
    hold(false, static_cast<int>(stats().fireInterval / Tick) + 2);
    hold(true, 1);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile), 6U);
}

TEST_F(WeaponTest, HoldingTheKeyChargesWithoutFiring)
{
    hold(true, static_cast<int>(stats().chargeTime / Tick) / 2);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile), 1U);
}

TEST_F(WeaponTest, PressesFasterThanTheFireRateAreIgnored)
{
    for (int i = 0; i < 4; ++i) {
        hold(true, 1);
        hold(false, 1);
    }
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile), 1U);
}

TEST_F(WeaponTest, ReleasingAfterTheChargeTimeFiresAChargedShot)
{
    hold(true, static_cast<int>(stats().chargeTime / Tick) + 2);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile), 1U);
    EXPECT_EQ(countType(registry(), rtype::EntityType::ChargedShot), 0U);
    hold(false, 1);
    EXPECT_EQ(countType(registry(), rtype::EntityType::ChargedShot), 1U);
    registry().each<server::Piercing, rtype::Projectile>(
        [this](engine::Entity, const server::Piercing &,
               const rtype::Projectile &projectile) {
            EXPECT_EQ(projectile.owner, ship());
        });
}

TEST_F(WeaponTest, ReleasingTooEarlyFiresNoChargedShot)
{
    hold(true, static_cast<int>(stats().chargeTime / Tick) / 2);
    hold(false, 1);
    EXPECT_EQ(countType(registry(), rtype::EntityType::ChargedShot), 0U);
}

TEST(Weapons, ReadsTheBoostDurations)
{
    std::string error;
    const auto stats = server::parseWeapons(
        R"({ "spreadDuration": 20, "burstShots": 3 })", error);

    ASSERT_TRUE(stats.has_value()) << error;
    const server::WeaponStats read = stats.value_or(server::WeaponStats{});
    EXPECT_FLOAT_EQ(read.durationOf(rtype::PowerUpKind::Spread).count(), 20.0F);
    EXPECT_FLOAT_EQ(read.durationOf(rtype::PowerUpKind::Aimed).count(), 45.0F);
    EXPECT_EQ(read.burstShots, 3);
    EXPECT_FALSE(
        server::parseWeapons(R"({ "aimedDuration": 300 })", error).has_value());
}

TEST(PowerUps, DroppedByDestroyedMediumEnemies)
{
    engine::Registry registry;
    server::EntityFactory factory;
    const engine::Entity medium = factory.create(
        registry, server::SpawnEnemy{.type = rtype::EnemyType::Medium,
                                     .position = {.x = 900.0F, .y = 400.0F}});
    const engine::Entity basic = factory.create(
        registry, server::SpawnEnemy{.type = rtype::EnemyType::Basic,
                                     .position = {.x = 900.0F, .y = 700.0F}});
    registry.get<rtype::Health>(medium).current = 0;
    registry.get<rtype::Health>(basic).current = 0;

    server::SpawnCommands commands;
    // Fixed seed: a failure can be replayed.
    // NOLINTNEXTLINE(cert-msc32-c,cert-msc51-cpp,bugprone-random-generator-seed)
    std::mt19937 random{1};
    server::powerUpDropSystem(registry, commands, random);
    server::spawnCommandSystem(registry, commands, factory);

    EXPECT_EQ(countType(registry, rtype::EntityType::PowerUp), 1U);
}

TEST(PowerUps, DropsEveryKindAtRandom)
{
    engine::Registry registry;
    server::EntityFactory factory;
    const engine::Entity medium = factory.create(
        registry, server::SpawnEnemy{.type = rtype::EnemyType::Medium});
    registry.get<rtype::Health>(medium).current = 0;
    // Fixed seed: a failure can be replayed.
    // NOLINTNEXTLINE(cert-msc32-c,cert-msc51-cpp,bugprone-random-generator-seed)
    std::mt19937 random{7};
    std::set<rtype::PowerUpKind> kinds;
    for (int i = 0; i < 100; ++i) {
        server::SpawnCommands commands;
        server::powerUpDropSystem(registry, commands, random);
        for (const server::SpawnCommand &command : commands) {
            if (const auto *drop =
                    std::get_if<server::SpawnPowerUp>(&command)) {
                kinds.insert(drop->kind);
            }
        }
    }
    EXPECT_EQ(kinds.size(), rtype::PowerUpKindCount);
}

TEST_F(WeaponTest, ASpreadBoostGivesThreeShotsThenFiveUpToTheMaximum)
{
    pick(rtype::PowerUpKind::Spread);
    EXPECT_EQ(registry().get<server::SpreadShot>(ship()).level, 2);
    for (int i = 0; i < 3; ++i) {
        pick(rtype::PowerUpKind::Spread);
    }
    EXPECT_EQ(registry().get<server::SpreadShot>(ship()).level,
              stats().maxLevel);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PowerUp), 0U);
}

TEST_F(WeaponTest, ABoostEndsAfterItsDuration)
{
    const float duration =
        stats().durationOf(rtype::PowerUpKind::Spread).count();
    pick(rtype::PowerUpKind::Spread);
    wait(duration - 1.0F);
    EXPECT_EQ(registry().get<server::SpreadShot>(ship()).level, 2);
    wait(2.0F);
    EXPECT_EQ(registry().get<server::SpreadShot>(ship()).level, 1);
    EXPECT_FLOAT_EQ(left(rtype::PowerUpKind::Spread), 0.0F);
}

TEST_F(WeaponTest, TakingABoostAgainRestartsItsTimer)
{
    pick(rtype::PowerUpKind::Aimed);
    wait(20.0F);
    pick(rtype::PowerUpKind::Aimed);
    EXPECT_FLOAT_EQ(left(rtype::PowerUpKind::Aimed),
                    stats().durationOf(rtype::PowerUpKind::Aimed).count());
}

TEST_F(WeaponTest, BoostsStackAndEndEachAtItsOwnTime)
{
    // Spread taken at 5:02, aimed at 5:22: both fire until the spread ends.
    pick(rtype::PowerUpKind::Spread);
    wait(20.0F);
    pick(rtype::PowerUpKind::Aimed);
    hold(true, 1);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile), 4U);

    wait(stats().durationOf(rtype::PowerUpKind::Spread).count() - 20.0F);
    EXPECT_EQ(registry().get<server::SpreadShot>(ship()).level, 1);
    EXPECT_TRUE(registry().has<server::AimedShot>(ship()));
    cool();
    hold(true, 1);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile), 6U);

    wait(stats().durationOf(rtype::PowerUpKind::Aimed).count());
    EXPECT_FALSE(registry().has<server::AimedShot>(ship()));
}

TEST_F(WeaponTest, AnAimedShotGoesTowardsTheNearestEnemy)
{
    server::EntityFactory factory;
    factory.create(registry(),
                   server::SpawnEnemy{.type = rtype::EnemyType::Basic,
                                      .position = {.x = 200.0F, .y = 900.0F}});
    registry().remove<server::SpreadShot>(ship());
    pick(rtype::PowerUpKind::Aimed);
    hold(true, 1);

    std::size_t aimedDown = 0;
    registry().each<rtype::Projectile, rtype::Velocity>(
        [&](engine::Entity, const rtype::Projectile &,
            const rtype::Velocity &velocity) {
            aimedDown += velocity.value.y > 0.0F ? 1U : 0U;
        });
    EXPECT_EQ(aimedDown, 1U);
}

TEST_F(WeaponTest, ABurstFiresEachShotAgainRightAfterIt)
{
    pick(rtype::PowerUpKind::Burst);
    hold(true, 1);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile), 1U);
    hold(true, static_cast<int>(stats().burstDelay / Tick) *
                   (stats().burstShots + 1));
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile),
              1U + stats().burstShots);
}

TEST_F(WeaponTest, EveryShipStartsWithASingleShotSpread)
{
    EXPECT_EQ(registry().get<server::SpreadShot>(ship()).level, 1);
}

TEST_F(WeaponTest, WithoutAShotStyleFiringMakesNoShot)
{
    registry().remove<server::SpreadShot>(ship());
    hold(true, 1);
    EXPECT_TRUE(registry().get<rtype::Weapon>(ship()).fired);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PlayerMissile), 0U);
}
