/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** player_control_system_test
*/

#include "common/components/InputState.hpp"
#include "common/components/Velocity.hpp"
#include "common/systems/PlayerControlSystem.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>

namespace {

// Runs the system on one player holding `actions` and returns its velocity.
engine::Vec2f velocityFor(std::uint8_t actions)
{
    engine::Registry registry;
    const engine::Entity player = registry.create();
    registry.emplace<rtype::InputState>(player,
                                        rtype::InputState{.actions = actions});
    registry.emplace<rtype::Velocity>(player);

    rtype::playerControlSystem(registry);
    return registry.get<rtype::Velocity>(player).value;
}

} // namespace

TEST(PlayerControlSystem, NoInputStopsTheShip)
{
    const engine::Vec2f velocity = velocityFor(0);

    EXPECT_FLOAT_EQ(velocity.x, 0.0F);
    EXPECT_FLOAT_EQ(velocity.y, 0.0F);
}

TEST(PlayerControlSystem, EachDirectionMovesAtPlayerSpeed)
{
    EXPECT_FLOAT_EQ(velocityFor(rtype::Action::Up).y, -rtype::playerSpeed);
    EXPECT_FLOAT_EQ(velocityFor(rtype::Action::Down).y, rtype::playerSpeed);
    EXPECT_FLOAT_EQ(velocityFor(rtype::Action::Left).x, -rtype::playerSpeed);
    EXPECT_FLOAT_EQ(velocityFor(rtype::Action::Right).x, rtype::playerSpeed);
}

TEST(PlayerControlSystem, OppositeDirectionsCancelOut)
{
    std::uint8_t actions = rtype::Action::Left | rtype::Action::Right;
    actions |= rtype::Action::Up;
    actions |= rtype::Action::Down;
    const engine::Vec2f velocity = velocityFor(actions);

    EXPECT_FLOAT_EQ(velocity.x, 0.0F);
    EXPECT_FLOAT_EQ(velocity.y, 0.0F);
}

TEST(PlayerControlSystem, DiagonalIsNotFaster)
{
    const engine::Vec2f velocity =
        velocityFor(rtype::Action::Up | rtype::Action::Right);

    EXPECT_GT(velocity.x, 0.0F);
    EXPECT_LT(velocity.y, 0.0F);
    EXPECT_FLOAT_EQ(std::hypot(velocity.x, velocity.y), rtype::playerSpeed);
}

TEST(PlayerControlSystem, FireDoesNotMove)
{
    const engine::Vec2f velocity = velocityFor(rtype::Action::Fire);

    EXPECT_FLOAT_EQ(velocity.x, 0.0F);
    EXPECT_FLOAT_EQ(velocity.y, 0.0F);
}
