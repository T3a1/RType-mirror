/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** chat_system_test
*/

#include "systems/ChatSystem.hpp"

#include <gtest/gtest.h>

#include <vector>

using rtype::protocol::ChatMessage;
using rtype::protocol::Payload;
using server::InboundKind;
using server::InboundMessage;

namespace {

// Client 3 plays in slot 2.
server::Players players() { return {{3, {.slot = 2}}}; }

} // namespace

TEST(ChatSystem, SendsTheLineToTheRoomWithTheSlotOfItsAuthor)
{
    server::Outbound outbound;

    server::chatSystem(
        {{.clientId = 3, .kind = InboundKind::Chat, .text = "hello"}},
        players(), outbound);

    const auto sent = outbound.drain();
    ASSERT_EQ(sent.size(), 1U);
    EXPECT_EQ(sent.front().to, server::Recipient::All);
    const Payload expected{ChatMessage{.slot = 2, .text = "hello"}};
    EXPECT_EQ(sent.front().payload, expected);
}

TEST(ChatSystem, DropsTheLinesOfUnknownClients)
{
    server::Outbound outbound;

    server::chatSystem(
        {{.clientId = 9, .kind = InboundKind::Chat, .text = "hello"}},
        players(), outbound);

    EXPECT_EQ(outbound.size(), 0U);
}

TEST(ChatSystem, IgnoresOtherMessages)
{
    server::Outbound outbound;
    const std::vector<InboundMessage> messages{
        {.clientId = 3, .kind = InboundKind::Input}};

    server::chatSystem(messages, players(), outbound);

    EXPECT_EQ(outbound.size(), 0U);
}
