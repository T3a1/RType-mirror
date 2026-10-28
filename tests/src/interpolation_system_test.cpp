/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** interpolation_system_test
*/

#include "common/components/Health.hpp"
#include "common/components/SnapshotHistory.hpp"
#include "common/components/Transform.hpp"
#include "common/systems/InterpolationSystem.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

#include <gtest/gtest.h>

namespace {

rtype::SnapshotHistory
historyOf(std::initializer_list<std::pair<std::uint32_t, float>> samples)
{
    rtype::SnapshotHistory history;
    for (const auto &[tick, x] : samples) {
        rtype::pushSample(history,
                          {.tick = tick, .position = {.x = x, .y = 0.0F}});
    }
    return history;
}

} // namespace

TEST(InterpolationSystem, PushKeepsSamplesInOrder)
{
    rtype::SnapshotHistory history = historyOf({{3, 1.0F}, {6, 2.0F}});
    rtype::pushSample(history, {.tick = 4, .position = {.x = 9.0F, .y = 0.0F}});
    rtype::pushSample(history, {.tick = 6, .position = {.x = 5.0F, .y = 0.0F}});

    ASSERT_EQ(history.count, 2U);
    EXPECT_EQ(history.samples.at(0).tick, 3U);
    EXPECT_EQ(history.samples.at(1).tick, 6U);
    EXPECT_FLOAT_EQ(history.samples.at(1).position.x, 5.0F);
}

TEST(InterpolationSystem, PushDropsTheOldestWhenFull)
{
    rtype::SnapshotHistory history;
    for (std::uint32_t tick = 1; tick <= rtype::SnapshotHistory::Capacity + 2;
         ++tick) {
        rtype::pushSample(history, {.tick = tick});
    }

    ASSERT_EQ(history.count, rtype::SnapshotHistory::Capacity);
    EXPECT_EQ(history.samples.front().tick, 3U);
    EXPECT_EQ(history.samples.back().tick,
              rtype::SnapshotHistory::Capacity + 2);
}

TEST(InterpolationSystem, InterpolatesBetweenSamples)
{
    const rtype::SnapshotHistory history =
        historyOf({{3, 0.0F}, {6, 30.0F}, {9, 60.0F}});

    EXPECT_FLOAT_EQ(rtype::samplePosition(history, 4.5).x, 15.0F);
    EXPECT_FLOAT_EQ(rtype::samplePosition(history, 8.0).x, 50.0F);
    EXPECT_FLOAT_EQ(rtype::samplePosition(history, 6.0).x, 30.0F);
}

TEST(InterpolationSystem, HoldsTheOldestSampleBeforeTheHistory)
{
    const rtype::SnapshotHistory history = historyOf({{3, 10.0F}, {6, 40.0F}});

    EXPECT_FLOAT_EQ(rtype::samplePosition(history, 1.0).x, 10.0F);
}

TEST(InterpolationSystem, HoldsASingleSample)
{
    const rtype::SnapshotHistory history = historyOf({{3, 10.0F}});

    EXPECT_FLOAT_EQ(rtype::samplePosition(history, 20.0).x, 10.0F);
}

TEST(InterpolationSystem, ExtrapolatesForALimitedTime)
{
    const rtype::SnapshotHistory history = historyOf({{3, 0.0F}, {6, 30.0F}});

    EXPECT_FLOAT_EQ(rtype::samplePosition(history, 9.0).x, 60.0F);
    EXPECT_FLOAT_EQ(rtype::samplePosition(history, 100.0).x,
                    30.0F + (rtype::MaxExtrapolation * 10.0));
}

TEST(InterpolationSystem, ClockStartsBehindTheFirstSnapshot)
{
    rtype::RenderClock clock;
    rtype::advanceRenderClock(clock, 0, engine::Duration{0.1F});
    EXPECT_FALSE(clock.started);

    rtype::advanceRenderClock(clock, 300, engine::Duration{0.1F});
    EXPECT_TRUE(clock.started);
    EXPECT_DOUBLE_EQ(clock.tick, 300.0 - rtype::InterpolationDelay);
}

TEST(InterpolationSystem, ClockAdvancesAtTheTickRate)
{
    rtype::RenderClock clock;
    rtype::advanceRenderClock(clock, 300, engine::Duration{0.0F});
    const double start = clock.tick;

    rtype::advanceRenderClock(clock, 303, engine::Duration{0.05F});

    EXPECT_NEAR(clock.tick, start + 3.0, 1e-4);
}

TEST(InterpolationSystem, ClockCatchesUpWithoutGoingBackwards)
{
    rtype::RenderClock clock;
    rtype::advanceRenderClock(clock, 300, engine::Duration{0.0F});
    double previous = clock.tick;

    for (int frame = 0; frame < 120; ++frame) {
        rtype::advanceRenderClock(clock, 300, engine::Duration{1.0F / 60.0F});
        EXPECT_GE(clock.tick, previous);
        previous = clock.tick;
    }
    EXPECT_LT(clock.tick, 300.0 + rtype::TickRate);
}

TEST(InterpolationSystem, ClockResyncsWhenFarOff)
{
    rtype::RenderClock clock;
    rtype::advanceRenderClock(clock, 300, engine::Duration{0.0F});

    rtype::advanceRenderClock(clock, 3000, engine::Duration{0.01F});

    EXPECT_DOUBLE_EQ(clock.tick, 3000.0 - rtype::InterpolationDelay);
}

TEST(InterpolationSystem, MovesEntitiesWithAHistory)
{
    engine::Registry registry;
    const engine::Entity synced = registry.create();
    registry.emplace<rtype::Transform>(synced, rtype::Transform{});
    registry.emplace<rtype::SnapshotHistory>(
        synced, historyOf({{3, 0.0F}, {6, 30.0F}}));
    const engine::Entity local = registry.create();
    registry.emplace<rtype::Transform>(
        local, rtype::Transform{.position = {.x = 7.0F, .y = 0.0F}});

    rtype::interpolationSystem(registry, 5.0);

    EXPECT_FLOAT_EQ(registry.get<rtype::Transform>(synced).position.x, 20.0F);
    EXPECT_FLOAT_EQ(registry.get<rtype::Transform>(local).position.x, 7.0F);
}

TEST(InterpolationSystem, HealthChangesAtTheTickOfItsSample)
{
    rtype::SnapshotHistory history;
    rtype::pushSample(history, {.tick = 3, .health = 30});
    rtype::pushSample(history, {.tick = 6, .health = 20});
    rtype::pushSample(history, {.tick = 9, .health = 10});

    EXPECT_EQ(rtype::sampleHealth(history, 1.0), 30);
    EXPECT_EQ(rtype::sampleHealth(history, 5.9), 30);
    EXPECT_EQ(rtype::sampleHealth(history, 6.0), 20);
    EXPECT_EQ(rtype::sampleHealth(history, 50.0), 10);
}

TEST(InterpolationSystem, AppliesHealthAtTheRenderTick)
{
    engine::Registry registry;
    const engine::Entity enemy = registry.create();
    registry.emplace<rtype::Transform>(enemy, rtype::Transform{});
    registry.emplace<rtype::Health>(enemy, rtype::Health{.current = 10});
    rtype::SnapshotHistory history;
    rtype::pushSample(history, {.tick = 3, .health = 30});
    rtype::pushSample(history, {.tick = 6, .health = 10});
    registry.emplace<rtype::SnapshotHistory>(enemy, history);

    rtype::interpolationSystem(registry, 4.0);

    EXPECT_EQ(registry.get<rtype::Health>(enemy).current, 30);
}

TEST(InterpolationSystem, DrawsAnEntityAheadByItsLead)
{
    engine::Registry registry;
    const engine::Entity missile = registry.create();
    registry.emplace<rtype::Transform>(missile, rtype::Transform{});
    rtype::SnapshotHistory history = historyOf({{3, 0.0F}, {9, 60.0F}});
    history.lead = 3.0;
    registry.emplace<rtype::SnapshotHistory>(missile, history);

    rtype::interpolationSystem(registry, 3.0);

    EXPECT_FLOAT_EQ(registry.get<rtype::Transform>(missile).position.x, 30.0F);
}
