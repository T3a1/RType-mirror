/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** boss_parts_test
*/

#include "BossLayout.hpp"
#include "EntityFactory.hpp"
#include "common/Playfield.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/Projectile.hpp"
#include "common/components/Transform.hpp"
#include "common/systems/MovementSystem.hpp"
#include "components/Boss.hpp"
#include "components/BossPart.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "systems/BossPartSystem.hpp"
#include "systems/BossSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/DamageSystem.hpp"
#include "systems/DeathSystem.hpp"
#include "systems/SpawnCommandSystem.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

namespace {

constexpr engine::Duration Tick{1.0F / 60.0F};

/** @brief One boss just past the right edge, and one player on the left,
 * run by the boss systems like the game loop does. */
class BossParts : public testing::Test {
  protected:
    void spawn(rtype::EnemyType type)
    {
        m_boss = m_factory.create(
            m_registry, server::SpawnEnemy{
                            .type = type,
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
            server::bossSystem(m_registry, m_commands, Tick);
            rtype::movementSystem(m_registry, Tick);
            server::bossPartSystem(m_registry, m_outbound, Tick);
            server::spawnCommandSystem(m_registry, m_commands, m_factory);
            m_registry.flush();
        }
    }

    /** @brief Fires a player shot right on an entity, and applies it. */
    void shootAt(engine::Entity target)
    {
        const auto at = m_registry.get<rtype::Transform>(target).position;
        m_factory.create(m_registry, server::SpawnProjectile{.position = at});
        server::damageSystem(m_registry, server::collisionSystem(m_registry),
                             m_outbound);
        server::deathSystem(m_registry, m_outbound);
        m_registry.flush();
    }

    /** @brief Returns the parts of the boss of a given look. */
    std::vector<engine::Entity> parts(rtype::EntityType look)
    {
        std::vector<engine::Entity> found;
        m_registry.each<server::BossPart, rtype::EntityType>(
            [&](engine::Entity entity, const server::BossPart &part,
                const rtype::EntityType &type) {
                if (part.owner == m_boss && type == look) {
                    found.push_back(entity);
                }
            });
        return found;
    }

    [[nodiscard]] int health() { return health(m_boss); }
    int health(engine::Entity entity)
    {
        return m_registry.get<rtype::Health>(entity).current;
    }

    engine::Registry &registry() { return m_registry; }
    [[nodiscard]] engine::Entity boss() const { return m_boss; }

  private:
    engine::Registry m_registry;
    server::EntityFactory m_factory;
    server::SpawnCommands m_commands;
    server::Outbound m_outbound;
    engine::Entity m_boss{};
};

TEST_F(BossParts, WalkerWalksOnTheGroundBothWays)
{
    spawn(rtype::EnemyType::Walker);
    run(8.0F);
    const auto &position = registry().get<rtype::Transform>(boss()).position;
    const float ground =
        rtype::PlayfieldHeight - (server::ENEMY_DEFS.at(13).size.y / 2.0F);
    EXPECT_FLOAT_EQ(position.y, ground);

    float leftmost = position.x;
    float rightmost = position.x;
    for (int i = 0; i < 40; ++i) {
        run(0.5F);
        const float x = registry().get<rtype::Transform>(boss()).position.x;
        leftmost = std::min(leftmost, x);
        rightmost = std::max(rightmost, x);
        EXPECT_FLOAT_EQ(registry().get<rtype::Transform>(boss()).position.y,
                        ground);
    }
    EXPECT_LT(leftmost, rtype::PlayfieldWidth * 0.5F);
    EXPECT_GT(rightmost, rtype::PlayfieldWidth * 0.75F);
}

TEST_F(BossParts, WalkerFiresFireballs)
{
    spawn(rtype::EnemyType::Walker);
    run(10.0F);
    std::size_t fireballs = 0;
    registry().each<rtype::Projectile, rtype::EntityType>(
        [&](engine::Entity, const rtype::Projectile &,
            const rtype::EntityType &type) {
            fireballs += type == rtype::EntityType::BossFireball ? 1U : 0U;
        });
    EXPECT_GT(fireballs, 0U);
}

TEST_F(BossParts, TwinHasAHalfOnTheCeilingThatSharesItsHealth)
{
    spawn(rtype::EnemyType::Twin);
    const auto tops = parts(rtype::EntityType::BossTwinTop);
    ASSERT_EQ(tops.size(), 1U);
    run(8.0F);

    const auto &bottom = registry().get<rtype::Transform>(boss()).position;
    const auto &top = registry().get<rtype::Transform>(tops.front()).position;
    EXPECT_FLOAT_EQ(top.x, bottom.x);
    EXPECT_FLOAT_EQ(top.y, server::TwinTopSize.y / 2.0F);
    EXPECT_FLOAT_EQ(bottom.y, rtype::PlayfieldHeight -
                                  (server::ENEMY_DEFS.at(14).size.y / 2.0F));

    const int before = health();
    shootAt(tops.front());
    EXPECT_EQ(health(), before - 50);
}

TEST_F(BossParts, BehemothTakesTripleDamageOnItsHead)
{
    spawn(rtype::EnemyType::Behemoth);
    const auto heads = parts(rtype::EntityType::BehemothHead);
    ASSERT_EQ(heads.size(), 1U);
    EXPECT_EQ(parts(rtype::EntityType::BehemothTail).size(),
              server::BehemothTailSegments - 1);
    ASSERT_EQ(parts(rtype::EntityType::BehemothTailTip).size(), 1U);
    run(6.0F);

    const int before = health();
    shootAt(heads.front());
    EXPECT_EQ(health(), before - 150);

    const int beforeTail = health();
    shootAt(parts(rtype::EntityType::BehemothTailTip).front());
    EXPECT_EQ(health(), beforeTail - 25);
}

TEST_F(BossParts, TailWavesAndFollowsTheBody)
{
    spawn(rtype::EnemyType::Behemoth);
    run(6.0F);
    const engine::Entity tip = parts(rtype::EntityType::BehemothTailTip)[0];
    const float first = registry().get<rtype::Transform>(tip).position.y -
                        registry().get<rtype::Transform>(boss()).position.y;
    run(0.7F);
    const float second = registry().get<rtype::Transform>(tip).position.y -
                         registry().get<rtype::Transform>(boss()).position.y;
    EXPECT_NE(first, second);
}

TEST_F(BossParts, PartsAreDestroyedWithTheirBoss)
{
    spawn(rtype::EnemyType::Behemoth);
    run(1.0F);
    registry().get<rtype::Health>(boss()).current = 1;
    shootAt(boss());
    EXPECT_FALSE(registry().alive(boss()));

    run(Tick.count() * 2.0F);
    EXPECT_TRUE(parts(rtype::EntityType::BehemothHead).empty());
    std::size_t left = 0;
    registry().each<server::BossPart>(
        [&](engine::Entity, const server::BossPart &) { ++left; });
    EXPECT_EQ(left, 0U);
}

} // namespace
