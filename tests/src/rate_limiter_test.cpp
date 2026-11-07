/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** rate_limiter_test
*/

#include "engine/RateLimiter.hpp"

#include <gtest/gtest.h>

#include <chrono>

using engine::RateLimiter;
using namespace std::chrono_literals;

TEST(RateLimiter, AcceptsABurstThenRefuses)
{
    RateLimiter limiter(3.0F, 1.0F);
    const RateLimiter::TimePoint now{};

    EXPECT_TRUE(limiter.allow(now));
    EXPECT_TRUE(limiter.allow(now));
    EXPECT_TRUE(limiter.allow(now));
    EXPECT_FALSE(limiter.allow(now));
}

TEST(RateLimiter, AcceptsAgainAsTimePasses)
{
    RateLimiter limiter(1.0F, 2.0F);
    const RateLimiter::TimePoint start{};

    EXPECT_TRUE(limiter.allow(start));
    EXPECT_FALSE(limiter.allow(start + 100ms));
    EXPECT_TRUE(limiter.allow(start + 600ms));
}

TEST(RateLimiter, NeverSavesMoreThanABurst)
{
    RateLimiter limiter(2.0F, 1.0F);
    const RateLimiter::TimePoint later = RateLimiter::TimePoint{} + 1h;

    EXPECT_TRUE(limiter.allow(RateLimiter::TimePoint{}));
    EXPECT_TRUE(limiter.allow(later));
    EXPECT_TRUE(limiter.allow(later));
    EXPECT_FALSE(limiter.allow(later));
}
