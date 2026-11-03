/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** pattern_system_test
*/

#include "common/components/MovementPattern.hpp"
#include "common/components/Player.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "common/systems/MovementSystem.hpp"
#include "common/systems/PatternSystem.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

namespace {

constexpr engine::Duration Tick{1.0F / 60.0F};

engine::Entity spawn(engine::Registry &registry,
                     const rtype::MovementPattern &pattern, float speed)
{
    const engine::Entity entity = registry.create();
    registry.emplace<rtype::Transform>(
        entity,
        rtype::Transform{.position = {.x = 1900.0F, .y = pattern.baseY}});
    registry.emplace<rtype::Velocity>(
        entity, rtype::Velocity{.value = {.x = -speed, .y = 0.0F}});
    registry.emplace<rtype::MovementPattern>(entity, pattern);
    return entity;
}

void run(engine::Registry &registry, int ticks)
{
    for (int i = 0; i < ticks; ++i) {
        rtype::patternSystem(registry, Tick);
        rtype::movementSystem(registry, Tick);
    }
}

} // namespace

TEST(PatternSystem, SineOffsetFollowsTheCurve)
{
    const rtype::MovementPattern sine{.kind = rtype::MovementKind::Sine,
                                      .amplitude = 100.0F,
                                      .frequency = 1.0F};

    EXPECT_NEAR(rtype::patternOffset(sine, engine::Duration{0.0F}), 0.0F, 1e-4);
    EXPECT_NEAR(rtype::patternOffset(sine, engine::Duration{0.25F}), 100.0F,
                1e-3);
    EXPECT_NEAR(rtype::patternOffset(sine, engine::Duration{0.75F}), -100.0F,
                1e-3);
}

TEST(PatternSystem, ZigzagIsATriangleWave)
{
    const rtype::MovementPattern zigzag{.kind = rtype::MovementKind::Zigzag,
                                        .amplitude = 100.0F,
                                        .frequency = 1.0F};

    EXPECT_NEAR(rtype::patternOffset(zigzag, engine::Duration{0.125F}), 50.0F,
                1e-2);
    EXPECT_NEAR(rtype::patternOffset(zigzag, engine::Duration{0.25F}), 100.0F,
                1e-2);
    EXPECT_NEAR(rtype::patternOffset(zigzag, engine::Duration{0.375F}), 50.0F,
                1e-2);
}

TEST(PatternSystem, PhaseShiftsTheCycle)
{
    const rtype::MovementPattern shifted{
        .kind = rtype::MovementKind::Sine,
        .amplitude = 100.0F,
        .frequency = 1.0F,
        .phase = std::numbers::pi_v<float> / 2.0F,
    };

    EXPECT_NEAR(rtype::patternOffset(shifted, engine::Duration{0.0F}), 100.0F,
                1e-3);
}

TEST(PatternSystem, EntityStaysOnItsSinePath)
{
    engine::Registry registry;
    const rtype::MovementPattern sine{.kind = rtype::MovementKind::Sine,
                                      .baseY = 500.0F,
                                      .amplitude = 100.0F,
                                      .frequency = 0.5F};
    const engine::Entity entity = spawn(registry, sine, 200.0F);

    run(registry, 30);

    const engine::Vec2f position =
        registry.get<rtype::Transform>(entity).position;
    EXPECT_NEAR(position.y, 500.0F + rtype::patternOffset(sine, Tick * 30.0F),
                1e-2);
    EXPECT_NEAR(position.x, 1900.0F - (200.0F * 0.5F), 1e-2);
}

TEST(PatternSystem, StraightComesBackToItsLine)
{
    engine::Registry registry;
    const engine::Entity entity =
        spawn(registry,
              rtype::MovementPattern{.kind = rtype::MovementKind::Straight,
                                     .baseY = 300.0F},
              100.0F);
    registry.get<rtype::Transform>(entity).position.y = 350.0F;

    run(registry, 1);

    EXPECT_NEAR(registry.get<rtype::Transform>(entity).position.y, 300.0F,
                1e-3);
}

TEST(PatternSystem, DiveRushesTowardTheNearestPlayer)
{
    engine::Registry registry;
    const engine::Entity far = registry.create();
    registry.emplace<rtype::Player>(far, rtype::Player{.slot = 1});
    registry.emplace<rtype::Transform>(
        far, rtype::Transform{.position = {.x = 100.0F, .y = 100.0F}});
    const engine::Entity near = registry.create();
    registry.emplace<rtype::Player>(near, rtype::Player{.slot = 2});
    registry.emplace<rtype::Transform>(
        near, rtype::Transform{.position = {.x = 1000.0F, .y = 900.0F}});

    const engine::Entity enemy =
        spawn(registry,
              rtype::MovementPattern{.kind = rtype::MovementKind::Dive,
                                     .baseY = 500.0F,
                                     .diveDelay = engine::Duration{0.5F},
                                     .diveSpeed = 600.0F},
              100.0F);

    run(registry, 29);
    EXPECT_FLOAT_EQ(registry.get<rtype::Velocity>(enemy).value.x, -100.0F);
    EXPECT_NEAR(registry.get<rtype::Transform>(enemy).position.y, 500.0F, 1e-3);

    run(registry, 2);
    const engine::Vec2f velocity = registry.get<rtype::Velocity>(enemy).value;
    EXPECT_NEAR(std::hypot(velocity.x, velocity.y), 600.0F, 1e-2);
    EXPECT_LT(velocity.x, 0.0F);
    EXPECT_GT(velocity.y, 0.0F);
}

TEST(PatternSystem, DiveKeepsItsDirection)
{
    engine::Registry registry;
    const engine::Entity player = registry.create();
    registry.emplace<rtype::Player>(player, rtype::Player{.slot = 1});
    registry.emplace<rtype::Transform>(
        player, rtype::Transform{.position = {.x = 100.0F, .y = 500.0F}});
    const engine::Entity enemy =
        spawn(registry,
              rtype::MovementPattern{.kind = rtype::MovementKind::Dive,
                                     .baseY = 500.0F,
                                     .diveSpeed = 600.0F},
              100.0F);

    run(registry, 1);
    const engine::Vec2f before = registry.get<rtype::Velocity>(enemy).value;
    registry.get<rtype::Transform>(player).position.y = 1000.0F;
    run(registry, 10);

    EXPECT_EQ(registry.get<rtype::Velocity>(enemy).value, before);
}

TEST(PatternSystem, DiveWithoutPlayersGoesLeft)
{
    engine::Registry registry;
    const engine::Entity enemy =
        spawn(registry,
              rtype::MovementPattern{.kind = rtype::MovementKind::Dive,
                                     .baseY = 500.0F,
                                     .diveSpeed = 600.0F},
              100.0F);

    run(registry, 1);

    const engine::Vec2f velocity = registry.get<rtype::Velocity>(enemy).value;
    EXPECT_FLOAT_EQ(velocity.x, -600.0F);
    EXPECT_FLOAT_EQ(velocity.y, 0.0F);
}

TEST(PatternSystem, ZeroTickChangesNothing)
{
    engine::Registry registry;
    const engine::Entity entity =
        spawn(registry,
              rtype::MovementPattern{.kind = rtype::MovementKind::Sine,
                                     .baseY = 500.0F,
                                     .amplitude = 100.0F,
                                     .frequency = 1.0F},
              100.0F);

    rtype::patternSystem(registry, engine::Duration{0.0F});

    EXPECT_FLOAT_EQ(registry.get<rtype::Velocity>(entity).value.y, 0.0F);
    EXPECT_FLOAT_EQ(
        registry.get<rtype::MovementPattern>(entity).elapsed.count(), 0.0F);
}
