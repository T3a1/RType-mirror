/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** vec2_test
*/

#include "engine/Vec2.hpp"

#include <gtest/gtest.h>

namespace {

engine::Vec2f vec(float x, float y) { return engine::Vec2f{.x = x, .y = y}; }

} // namespace

TEST(Vec2, DefaultsToZero)
{
    const engine::Vec2f v;

    EXPECT_EQ(v, vec(0.0F, 0.0F));
}

TEST(Vec2, Arithmetic)
{
    const engine::Vec2f a = vec(1.0F, 2.0F);
    const engine::Vec2f b = vec(3.0F, 5.0F);

    EXPECT_EQ(a + b, vec(4.0F, 7.0F));
    EXPECT_EQ(b - a, vec(2.0F, 3.0F));
    EXPECT_EQ(a * 2.0F, vec(2.0F, 4.0F));
    EXPECT_EQ(2.0F * a, vec(2.0F, 4.0F));
}

TEST(Vec2, CompoundAssignment)
{
    engine::Vec2f v = vec(1.0F, 1.0F);

    v += vec(1.0F, 2.0F);
    v -= vec(0.5F, 0.5F);
    v *= 2.0F;

    EXPECT_EQ(v, vec(3.0F, 5.0F));
}
