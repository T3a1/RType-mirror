/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** sparse_set_test
*/

#include "engine/Entity.hpp"
#include "engine/SparseSet.hpp"

#include <gtest/gtest.h>

namespace {

struct Score {
    int value = 0;
};

engine::Entity entity(std::uint32_t id, std::uint32_t generation = 0)
{
    return engine::Entity{.id = id, .generation = generation};
}

} // namespace

TEST(SparseSet, StartsEmpty)
{
    const engine::SparseSet<Score> set;

    EXPECT_EQ(set.size(), 0U);
    EXPECT_FALSE(set.contains(entity(0)));
}

TEST(SparseSet, EmplaceStoresTheComponent)
{
    engine::SparseSet<Score> set;

    set.emplace(entity(3), Score{.value = 42});

    EXPECT_EQ(set.size(), 1U);
    EXPECT_TRUE(set.contains(entity(3)));
    EXPECT_EQ(set.get(entity(3)).value, 42);
}

TEST(SparseSet, EmplaceTwiceReplacesTheComponent)
{
    engine::SparseSet<Score> set;

    set.emplace(entity(1), Score{.value = 1});
    set.emplace(entity(1), Score{.value = 2});

    EXPECT_EQ(set.size(), 1U);
    EXPECT_EQ(set.get(entity(1)).value, 2);
}

TEST(SparseSet, OtherGenerationIsNotContained)
{
    engine::SparseSet<Score> set;

    set.emplace(entity(0, 0));

    EXPECT_FALSE(set.contains(entity(0, 1)));
}

TEST(SparseSet, RemoveKeepsTheOtherEntities)
{
    engine::SparseSet<Score> set;
    set.emplace(entity(0), Score{.value = 10});
    set.emplace(entity(1), Score{.value = 11});
    set.emplace(entity(2), Score{.value = 12});

    // Removing the middle one moves the last one into its slot.
    set.remove(entity(1));

    EXPECT_EQ(set.size(), 2U);
    EXPECT_FALSE(set.contains(entity(1)));
    EXPECT_EQ(set.get(entity(0)).value, 10);
    EXPECT_EQ(set.get(entity(2)).value, 12);
}

TEST(SparseSet, RemoveLastAndMissingEntities)
{
    engine::SparseSet<Score> set;
    set.emplace(entity(0));
    set.emplace(entity(1));

    set.remove(entity(1));
    set.remove(entity(1));
    set.remove(entity(7));

    EXPECT_EQ(set.size(), 1U);
    EXPECT_TRUE(set.contains(entity(0)));
}

TEST(SparseSet, TryGet)
{
    engine::SparseSet<Score> set;
    set.emplace(entity(0), Score{.value = 5});

    ASSERT_NE(set.tryGet(entity(0)), nullptr);
    EXPECT_EQ(set.tryGet(entity(0))->value, 5);
    EXPECT_EQ(set.tryGet(entity(1)), nullptr);
}

TEST(SparseSet, DensePositionsMatchEntitiesAndData)
{
    engine::SparseSet<Score> set;
    set.emplace(entity(4), Score{.value = 40});
    set.emplace(entity(9), Score{.value = 90});

    for (std::size_t i = 0; i < set.size(); ++i) {
        EXPECT_EQ(set.dataAt(i).value,
                  static_cast<int>(set.entityAt(i).id) * 10);
    }
}
