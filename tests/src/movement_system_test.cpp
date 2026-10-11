/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** movement_system_test
*/

#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "common/systems/MovementSystem.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

#include <gtest/gtest.h>

TEST(MovementSystem, MovesByVelocityTimesDt)
{
    engine::Registry registry;
    const engine::Entity entity = registry.create();
    registry.emplace<rtype::Transform>(
        entity, rtype::Transform{.position = {.x = 10.0F, .y = 20.0F}});
    registry.emplace<rtype::Velocity>(
        entity, rtype::Velocity{.value = {.x = 100.0F, .y = -40.0F}});

    rtype::movementSystem(registry, engine::Duration{0.5F});

    const engine::Vec2f position =
        registry.get<rtype::Transform>(entity).position;
    EXPECT_FLOAT_EQ(position.x, 60.0F);
    EXPECT_FLOAT_EQ(position.y, 0.0F);
}

TEST(MovementSystem, IgnoresEntitiesWithoutVelocity)
{
    engine::Registry registry;
    const engine::Entity entity = registry.create();
    registry.emplace<rtype::Transform>(
        entity, rtype::Transform{.position = {.x = 10.0F, .y = 20.0F}});

    rtype::movementSystem(registry, engine::Duration{1.0F});

    const engine::Vec2f position =
        registry.get<rtype::Transform>(entity).position;
    EXPECT_FLOAT_EQ(position.x, 10.0F);
    EXPECT_FLOAT_EQ(position.y, 20.0F);
}
