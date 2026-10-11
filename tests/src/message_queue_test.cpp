/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** message_queue_test
*/

#include "engine/MessageQueue.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <thread>
#include <vector>

TEST(MessageQueue, DrainReturnsMessagesInOrderAndEmptiesTheQueue)
{
    engine::MessageQueue<int> queue;
    EXPECT_TRUE(queue.push(1));
    EXPECT_TRUE(queue.push(2));
    EXPECT_TRUE(queue.push(3));

    EXPECT_EQ(queue.drain(), (std::vector<int>{1, 2, 3}));
    EXPECT_EQ(queue.size(), 0U);
    EXPECT_TRUE(queue.drain().empty());
}

TEST(MessageQueue, RefusesMessagesPastItsCapacity)
{
    engine::MessageQueue<int> queue(2);
    EXPECT_TRUE(queue.push(1));
    EXPECT_TRUE(queue.push(2));
    EXPECT_FALSE(queue.push(3));

    EXPECT_EQ(queue.drain(), (std::vector<int>{1, 2}));
    EXPECT_TRUE(queue.push(4));
}

TEST(MessageQueue, FailedPushLeavesTheMessageUntouched)
{
    engine::MessageQueue<std::vector<int>> queue(0);
    std::vector<int> message{1, 2, 3};

    EXPECT_FALSE(queue.push(std::move(message)));
    // NOLINTNEXTLINE(bugprone-use-after-move)
    EXPECT_EQ(message, (std::vector<int>{1, 2, 3}));
}

TEST(MessageQueue, WaitPopReturnsAMessagePushedByAnotherThread)
{
    engine::MessageQueue<int> queue;
    std::thread producer([&queue] { queue.push(42); });

    EXPECT_EQ(queue.waitPop(), std::optional<int>{42});
    producer.join();
}

TEST(MessageQueue, CloseWakesAWaitingThread)
{
    engine::MessageQueue<int> queue;
    std::optional<int> result = 0;
    std::thread consumer([&queue, &result] { result = queue.waitPop(); });

    queue.close();
    consumer.join();
    EXPECT_EQ(result, std::nullopt);
}

TEST(MessageQueue, ClosedQueueRefusesPushesButKeepsWaitingMessages)
{
    engine::MessageQueue<int> queue;
    queue.push(1);
    queue.push(2);
    queue.close();

    EXPECT_TRUE(queue.closed());
    EXPECT_FALSE(queue.push(3));
    EXPECT_EQ(queue.waitPop(), std::optional<int>{1});
    EXPECT_EQ(queue.waitPop(), std::optional<int>{2});
    EXPECT_EQ(queue.waitPop(), std::nullopt);
}
