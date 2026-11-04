/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** instance_manager_test
*/

#include "FakeNetwork.hpp"
#include "engine/IInstance.hpp"
#include "engine/InstanceManager.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {

// What the instances of a test share with it. Generic messages: the manager
// works with any game.
struct Counters {
    std::atomic<int> created{0};
    std::atomic<int> stopped{0};
    std::atomic<int> ticks{0};
    std::atomic<bool> fail{false};
    std::atomic<bool> joinable{true};
};

class CountingInstance final : public engine::IInstance {
  public:
    explicit CountingInstance(Counters &counters) : m_counters(counters) {}

    void tick(engine::Duration /*dt*/) override
    {
        ++m_counters.ticks;
        if (m_counters.fail) {
            throw std::runtime_error("test failure");
        }
    }

    void onStop() override { ++m_counters.stopped; }

    [[nodiscard]] bool joinable() const override { return m_counters.joinable; }

  private:
    Counters &m_counters;
};

using Manager = engine::InstanceManager<int, int>;

Manager makeManager(Counters &counters, std::size_t maxInstances = 2,
                    std::size_t maxMembers = 2)
{
    return {
        [&counters](Manager::Link & /*link*/) {
            ++counters.created;
            return std::make_unique<CountingInstance>(counters);
        },
        {.maxInstances = maxInstances, .maxMembers = maxMembers},
        std::chrono::milliseconds(1),
    };
}

// The membership of a join that must succeed.
Manager::Membership joined(Manager &manager, engine::InstanceId id)
{
    const auto membership = manager.join(id);
    EXPECT_TRUE(membership.has_value());
    return membership.value_or(Manager::Membership{});
}

// The instances that failed, once at least one did.
std::vector<engine::InstanceId> waitForFailure(Manager &manager)
{
    std::vector<engine::InstanceId> failed;
    EXPECT_TRUE(tests::eventually([&] {
        failed = manager.reap();
        return !failed.empty();
    }));
    return failed;
}

} // namespace

TEST(InstanceManager, CreatesAnInstanceForItsFirstMember)
{
    Counters counters;
    Manager manager = makeManager(counters);

    const Manager::Membership first = joined(manager, 7);
    const Manager::Membership second = joined(manager, 7);

    EXPECT_EQ(first.id, 7U);
    EXPECT_EQ(first.link, second.link);
    EXPECT_EQ(counters.created, 1);
    EXPECT_EQ(manager.size(), 1U);
}

TEST(InstanceManager, InstancesHaveTheirOwnQueues)
{
    Counters counters;
    Manager manager = makeManager(counters);

    const Manager::Membership first = joined(manager, 1);
    const Manager::Membership second = joined(manager, 2);

    ASSERT_NE(first.link, second.link);
    ASSERT_NE(first.link, nullptr);
    ASSERT_NE(second.link, nullptr);
    first.link->inbound.push(42);
    EXPECT_EQ(second.link->inbound.size(), 0U);
}

TEST(InstanceManager, AnyInstanceFillsTheFirstWithRoomThenCreatesAnother)
{
    Counters counters;
    Manager manager = makeManager(counters);

    EXPECT_EQ(joined(manager, engine::AnyInstance).id, 1U);
    EXPECT_EQ(joined(manager, engine::AnyInstance).id, 1U);
    EXPECT_EQ(joined(manager, engine::AnyInstance).id, 2U);
}

TEST(InstanceManager, RefusesAFullInstanceAndTooManyInstances)
{
    Counters counters;
    Manager manager = makeManager(counters, 1, 1);

    EXPECT_TRUE(manager.join(1).has_value());
    EXPECT_FALSE(manager.join(1).has_value());
    EXPECT_FALSE(manager.join(2).has_value());
    EXPECT_FALSE(manager.join(engine::AnyInstance).has_value());
    EXPECT_EQ(counters.created, 1);
}

TEST(InstanceManager, RefusesAnInstanceThatIsNotJoinable)
{
    Counters counters;
    Manager manager = makeManager(counters);
    ASSERT_TRUE(manager.join(1).has_value());

    counters.joinable = false;

    EXPECT_FALSE(manager.join(1).has_value());
    EXPECT_FALSE(manager.join(2).has_value());
    EXPECT_EQ(manager.size(), 1U);
}

TEST(InstanceManager, StopsAnInstanceAfterItsLastMember)
{
    Counters counters;
    Manager manager = makeManager(counters);
    ASSERT_TRUE(manager.join(1).has_value());
    ASSERT_TRUE(manager.join(1).has_value());

    manager.leave(1);
    EXPECT_EQ(manager.size(), 1U);
    EXPECT_EQ(counters.stopped, 0);

    manager.leave(1);
    EXPECT_EQ(manager.size(), 0U);
    EXPECT_TRUE(tests::eventually([&] {
        EXPECT_TRUE(manager.reap().empty());
        return counters.stopped == 1;
    }));
}

TEST(InstanceManager, TheIdOfAStoppedInstanceCanBeUsedAgain)
{
    Counters counters;
    Manager manager = makeManager(counters);
    ASSERT_TRUE(manager.join(1).has_value());
    manager.leave(1);

    EXPECT_TRUE(manager.join(1).has_value());
    EXPECT_EQ(counters.created, 2);
}

TEST(InstanceManager, RunsTheInstancesAtTheFixedStep)
{
    Counters counters;
    Manager manager = makeManager(counters);
    ASSERT_TRUE(manager.join(1).has_value());

    EXPECT_TRUE(tests::eventually([&] { return counters.ticks > 3; }));
}

TEST(InstanceManager, AFailingInstanceOnlyEndsItself)
{
    Counters failing;
    Counters healthy;
    Manager manager(
        [&, count = 0](Manager::Link & /*link*/) mutable {
            return std::make_unique<CountingInstance>(count++ == 0 ? failing
                                                                   : healthy);
        },
        {.maxInstances = 2, .maxMembers = 2}, std::chrono::milliseconds(1));
    ASSERT_TRUE(manager.join(1).has_value());
    ASSERT_TRUE(manager.join(2).has_value());

    failing.fail = true;

    EXPECT_EQ(waitForFailure(manager), std::vector<engine::InstanceId>{1});
    EXPECT_EQ(manager.size(), 1U);
    const int ticks = healthy.ticks;
    EXPECT_TRUE(tests::eventually([&] { return healthy.ticks > ticks; }));
}

TEST(InstanceManager, RefusesToJoinWhenTheFactoryFails)
{
    Manager manager(
        [](Manager::Link & /*link*/) -> std::unique_ptr<engine::IInstance> {
            throw std::runtime_error("test failure");
        },
        {}, std::chrono::milliseconds(1));

    EXPECT_FALSE(manager.join(1).has_value());
    EXPECT_EQ(manager.size(), 0U);
}
