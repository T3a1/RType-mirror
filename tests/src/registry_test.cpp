/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** registry_test
*/

#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

#include <gtest/gtest.h>

#include <vector>

namespace {

struct Position {
    int x = 0;
};

struct Speed {
    int dx = 0;
};

} // namespace

TEST(Registry, CreateGivesDistinctAliveEntities)
{
    engine::Registry registry;

    const engine::Entity a = registry.create();
    const engine::Entity b = registry.create();

    EXPECT_NE(a, b);
    EXPECT_TRUE(registry.alive(a));
    EXPECT_TRUE(registry.alive(b));
}

TEST(Registry, UnknownEntityIsNotAlive)
{
    const engine::Registry registry;

    EXPECT_FALSE(registry.alive(engine::Entity{.id = 42, .generation = 0}));
}

TEST(Registry, DestroyWaitsForFlush)
{
    engine::Registry registry;
    const engine::Entity e = registry.create();
    registry.emplace<Position>(e);

    registry.destroy(e);

    EXPECT_TRUE(registry.alive(e));
    EXPECT_TRUE(registry.has<Position>(e));

    registry.flush();

    EXPECT_FALSE(registry.alive(e));
    EXPECT_FALSE(registry.has<Position>(e));
}

TEST(Registry, FlushRecyclesTheIdWithANewGeneration)
{
    engine::Registry registry;
    const engine::Entity old = registry.create();
    registry.destroy(old);
    registry.flush();

    const engine::Entity recycled = registry.create();

    EXPECT_EQ(recycled.id, old.id);
    EXPECT_NE(recycled.generation, old.generation);
    EXPECT_TRUE(registry.alive(recycled));
    EXPECT_FALSE(registry.alive(old));
}

TEST(Registry, StaleHandleCannotDestroyTheNewEntity)
{
    engine::Registry registry;
    const engine::Entity old = registry.create();
    registry.destroy(old);
    registry.flush();
    const engine::Entity recycled = registry.create();

    registry.destroy(old);
    registry.flush();

    EXPECT_TRUE(registry.alive(recycled));
}

TEST(Registry, DestroyingTwiceFreesTheIdOnce)
{
    engine::Registry registry;
    const engine::Entity e = registry.create();

    registry.destroy(e);
    registry.destroy(e);
    registry.flush();

    const engine::Entity a = registry.create();
    const engine::Entity b = registry.create();
    EXPECT_NE(a.id, b.id);
}

TEST(Registry, ComponentAccess)
{
    engine::Registry registry;
    const engine::Entity e = registry.create();

    registry.emplace<Position>(e, Position{.x = 3});

    EXPECT_TRUE(registry.has<Position>(e));
    EXPECT_FALSE(registry.has<Speed>(e));
    EXPECT_EQ(registry.get<Position>(e).x, 3);
    EXPECT_EQ(registry.tryGet<Speed>(e), nullptr);

    registry.get<Position>(e).x = 4;
    ASSERT_NE(registry.tryGet<Position>(e), nullptr);
    EXPECT_EQ(registry.tryGet<Position>(e)->x, 4);

    registry.remove<Position>(e);
    EXPECT_FALSE(registry.has<Position>(e));
    EXPECT_TRUE(registry.alive(e));
}

TEST(Registry, EachVisitsOnlyEntitiesWithEveryComponent)
{
    engine::Registry registry;
    const engine::Entity moving = registry.create();
    const engine::Entity still = registry.create();
    registry.emplace<Position>(moving);
    registry.emplace<Speed>(moving);
    registry.emplace<Position>(still);

    std::vector<engine::Entity> visited;
    registry.each<Position, Speed>(
        [&visited](engine::Entity e, Position &, Speed &) {
            visited.push_back(e);
        });

    ASSERT_EQ(visited.size(), 1U);
    EXPECT_EQ(visited[0], moving);
}

TEST(Registry, EachCanModifyComponents)
{
    engine::Registry registry;
    const engine::Entity e = registry.create();
    registry.emplace<Position>(e, Position{.x = 1});
    registry.emplace<Speed>(e, Speed{.dx = 2});

    registry.each<Position, Speed>(
        [](engine::Entity, Position &position, Speed &speed) {
            position.x += speed.dx;
        });

    EXPECT_EQ(registry.get<Position>(e).x, 3);
}

TEST(Registry, DestroyInsideEachVisitsEveryEntity)
{
    engine::Registry registry;
    for (int i = 0; i < 3; ++i) {
        registry.emplace<Position>(registry.create());
    }

    int visited = 0;
    registry.each<Position>([&](engine::Entity e, Position &) {
        ++visited;
        registry.destroy(e);
    });
    registry.flush();

    EXPECT_EQ(visited, 3);
    int left = 0;
    registry.each<Position>([&left](engine::Entity, Position &) { ++left; });
    EXPECT_EQ(left, 0);
}
