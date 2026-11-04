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
#include "common/components/Projectile.hpp"
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

#include <gtest/gtest.h>

#include <cstddef>
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
            server::shootSystem(m_registry, m_factory, Tick, m_stats);
            server::spreadShotSystem(m_registry, m_factory, m_stats);
            m_registry.flush();
        }
    }

    engine::Registry &registry() { return m_registry; }
    [[nodiscard]] engine::Entity ship() const { return m_ship; }
    [[nodiscard]] const server::WeaponStats &stats() const { return m_stats; }

  private:
    engine::Registry m_registry;
    server::EntityFactory m_factory;
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

    server::powerUpDropSystem(registry, factory);

    EXPECT_EQ(countType(registry, rtype::EntityType::PowerUp), 1U);
}

TEST_F(WeaponTest, APowerUpRaisesTheWeaponLevelUpToTheMaximum)
{
    server::EntityFactory factory;
    server::Outbound outbound;
    for (int i = 0; i < 4; ++i) {
        const engine::Entity powerUp =
            factory.create(registry(), server::SpawnPowerUp{});
        server::pickupSystem(registry(), {{.a = ship(), .b = powerUp}},
                             outbound, stats().maxLevel);
        registry().flush();
    }

    EXPECT_EQ(registry().get<server::SpreadShot>(ship()).level,
              stats().maxLevel);
    EXPECT_EQ(countType(registry(), rtype::EntityType::PowerUp), 0U);
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
