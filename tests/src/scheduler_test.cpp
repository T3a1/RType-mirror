/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** scheduler_test
*/

#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "engine/Scheduler.hpp"

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

namespace {

// Builds a system that appends its name to the log.
engine::System logger(std::vector<std::string> &log, std::string name)
{
    return [&log, name = std::move(name)](
               engine::Registry &, engine::Duration) { log.push_back(name); };
}

} // namespace

TEST(Scheduler, RunsStagesInOrderWhateverTheAddOrder)
{
    engine::Scheduler scheduler;
    engine::Registry registry;
    std::vector<std::string> log;

    scheduler.add(engine::Stage::Output, "output", logger(log, "output"));
    scheduler.add(engine::Stage::Input, "input", logger(log, "input"));
    scheduler.add(engine::Stage::Cleanup, "cleanup", logger(log, "cleanup"));
    scheduler.add(engine::Stage::Simulation, "sim", logger(log, "sim"));
    scheduler.run(registry, engine::Duration{0.016F});

    EXPECT_EQ(log,
              (std::vector<std::string>{"input", "sim", "cleanup", "output"}));
}

TEST(Scheduler, KeepsTheAddOrderWithinAStage)
{
    engine::Scheduler scheduler;
    engine::Registry registry;
    std::vector<std::string> log;

    scheduler.add(engine::Stage::Simulation, "a", logger(log, "a"));
    scheduler.add(engine::Stage::Simulation, "b", logger(log, "b"));
    scheduler.add(engine::Stage::Simulation, "c", logger(log, "c"));
    scheduler.run(registry, engine::Duration{0.016F});

    EXPECT_EQ(log, (std::vector<std::string>{"a", "b", "c"}));
}

TEST(Scheduler, RunsASystemOnceEveryNTicks)
{
    engine::Scheduler scheduler;
    engine::Registry registry;
    int runs = 0;

    scheduler.add(engine::Stage::Output, "snapshot",
                  [&runs](engine::Registry &, engine::Duration) { ++runs; },
                  {.every = 3});
    for (int i = 0; i < 9; ++i) {
        scheduler.run(registry, engine::Duration{0.016F});
    }

    EXPECT_EQ(runs, 3);
}

TEST(Scheduler, TreatsEveryZeroAsEveryTick)
{
    engine::Scheduler scheduler;
    engine::Registry registry;
    int runs = 0;

    scheduler.add(engine::Stage::Simulation, "zero",
                  [&runs](engine::Registry &, engine::Duration) { ++runs; },
                  {.every = 0});
    scheduler.run(registry, engine::Duration{0.016F});
    scheduler.run(registry, engine::Duration{0.016F});

    EXPECT_EQ(runs, 2);
}

TEST(Scheduler, FlushesAfterCleanupBeforeOutput)
{
    engine::Scheduler scheduler;
    engine::Registry registry;
    const engine::Entity entity = registry.create();
    bool aliveInCleanup = false;
    bool aliveInOutput = true;

    scheduler.add(
        engine::Stage::Simulation, "kill",
        [entity](engine::Registry &r, engine::Duration) { r.destroy(entity); });
    scheduler.add(engine::Stage::Cleanup, "check",
                  [&](engine::Registry &r, engine::Duration) {
                      aliveInCleanup = r.alive(entity);
                  });
    scheduler.add(engine::Stage::Output, "check",
                  [&](engine::Registry &r, engine::Duration) {
                      aliveInOutput = r.alive(entity);
                  });
    scheduler.run(registry, engine::Duration{0.016F});

    EXPECT_TRUE(aliveInCleanup);
    EXPECT_FALSE(aliveInOutput);
}

TEST(Scheduler, PassesDtToTheSystems)
{
    engine::Scheduler scheduler;
    engine::Registry registry;
    engine::Duration received{0.0F};

    scheduler.add(engine::Stage::Simulation, "dt",
                  [&received](engine::Registry &, engine::Duration dt) {
                      received = dt;
                  });
    scheduler.run(registry, engine::Duration{0.5F});

    EXPECT_FLOAT_EQ(received.count(), 0.5F);
}

TEST(Scheduler, RunsASystemOnlyWhileItsConditionHolds)
{
    engine::Scheduler scheduler;
    engine::Registry registry;
    bool enabled = false;
    int runs = 0;

    scheduler.add(engine::Stage::Simulation, "gated",
                  [&runs](engine::Registry &, engine::Duration) { ++runs; },
                  {.when = [&enabled] { return enabled; }});
    scheduler.run(registry, engine::Duration{0.016F});
    EXPECT_EQ(runs, 0);

    enabled = true;
    scheduler.run(registry, engine::Duration{0.016F});
    EXPECT_EQ(runs, 1);
}

TEST(Scheduler, ChecksAConditionAfterTheEarlierSystemsOfTheTick)
{
    engine::Scheduler scheduler;
    engine::Registry registry;
    bool enabled = false;
    int runs = 0;

    scheduler.add(
        engine::Stage::Input, "enable",
        [&enabled](engine::Registry &, engine::Duration) { enabled = true; });
    scheduler.add(engine::Stage::Simulation, "gated",
                  [&runs](engine::Registry &, engine::Duration) { ++runs; },
                  {.when = [&enabled] { return enabled; }});
    scheduler.run(registry, engine::Duration{0.016F});

    EXPECT_EQ(runs, 1);
}

TEST(Scheduler, CombinesEveryAndWhen)
{
    engine::Scheduler scheduler;
    engine::Registry registry;
    int runs = 0;
    int checks = 0;

    scheduler.add(engine::Stage::Output, "both",
                  [&runs](engine::Registry &, engine::Duration) { ++runs; },
                  {.every = 2, .when = [&checks] {
                       ++checks;
                       return checks > 1;
                   }});
    for (int i = 0; i < 6; ++i) {
        scheduler.run(registry, engine::Duration{0.016F});
    }

    EXPECT_EQ(checks, 3);
    EXPECT_EQ(runs, 2);
}
