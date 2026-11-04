/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** boss_test
*/

#include "EntityFactory.hpp"
#include "common/Playfield.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Health.hpp"
#include "common/components/Player.hpp"
#include "common/components/Projectile.hpp"
#include "common/components/Transform.hpp"
#include "common/systems/MovementSystem.hpp"
#include "components/Boss.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "systems/BossSystem.hpp"

#include <gtest/gtest.h>

#include <cstddef>

namespace {

constexpr engine::Duration Tick{1.0F / 60.0F};

std::size_t countShots(engine::Registry &registry)
{
    std::size_t count = 0;
    registry.each<rtype::Projectile>(
        [&count](engine::Entity, const rtype::Projectile &) { ++count; });
    return count;
}

/** @brief A boss just past the right edge, and one player on the left. */
class BossTest : public testing::Test {
  protected:
    BossTest()
    {
        m_boss = m_factory.create(
            m_registry, server::SpawnEnemy{
                            .type = rtype::EnemyType::Boss,
                            .position = {.x = rtype::PlayfieldWidth + 200.0F,
                                         .y = rtype::PlayfieldHeight / 2.0F}});
        m_factory.create(
            m_registry, server::SpawnPlayer{
                            .slot = 1, .position = {.x = 200.0F, .y = 540.0F}});
    }

    void run(float seconds)
    {
        const auto ticks = static_cast<int>(seconds / Tick.count());
        for (int i = 0; i < ticks; ++i) {
            server::bossSystem(m_registry, m_factory, Tick);
            rtype::movementSystem(m_registry, Tick);
            m_registry.flush();
        }
    }

    engine::Registry &registry() { return m_registry; }
    [[nodiscard]] engine::Entity boss() const { return m_boss; }

  private:
    engine::Registry m_registry;
    server::EntityFactory m_factory;
    engine::Entity m_boss{};
};

} // namespace

TEST(BossPhase, DropsWithHealth)
{
    EXPECT_EQ(server::bossPhase(3000, 3000), 1);
    EXPECT_EQ(server::bossPhase(2001, 3000), 1);
    EXPECT_EQ(server::bossPhase(2000, 3000), 2);
    EXPECT_EQ(server::bossPhase(1001, 3000), 2);
    EXPECT_EQ(server::bossPhase(1000, 3000), 3);
    EXPECT_EQ(server::bossPhase(0, 3000), 3);
}

TEST_F(BossTest, StartsWithFullHealthAndNoPath)
{
    const auto &health = registry().get<rtype::Health>(boss());
    EXPECT_EQ(health.current, rtype::BossMaxHealth);
    EXPECT_EQ(health.max, rtype::BossMaxHealth);
    EXPECT_FALSE(registry().has<rtype::MovementPattern>(boss()));
}

TEST_F(BossTest, ComesInStopsAndMovesUpAndDown)
{
    run(4.0F);
    const float x = registry().get<rtype::Transform>(boss()).position.x;
    EXPECT_LT(x, rtype::PlayfieldWidth);
    EXPECT_TRUE(registry().get<server::Boss>(boss()).arrived);
    const float y = registry().get<rtype::Transform>(boss()).position.y;

    run(1.0F);
    const auto &position = registry().get<rtype::Transform>(boss()).position;
    EXPECT_FLOAT_EQ(position.x, x);
    EXPECT_NE(position.y, y);
    EXPECT_GE(position.y, 0.0F);
    EXPECT_LE(position.y, rtype::PlayfieldHeight);
}

TEST_F(BossTest, ShootsMoreInLaterPhases)
{
    run(4.0F); // arrive
    const std::size_t before = countShots(registry());
    run(2.0F);
    const std::size_t phaseOne = countShots(registry()) - before;
    EXPECT_GT(phaseOne, 0U);

    registry().get<rtype::Health>(boss()).current = rtype::BossMaxHealth / 4;
    const std::size_t middle = countShots(registry());
    run(2.0F);
    const std::size_t phaseThree = countShots(registry()) - middle;
    EXPECT_EQ(registry().get<server::Boss>(boss()).phase, 3);
    EXPECT_GT(phaseThree, phaseOne * 3);
}
