/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** chat_test
*/

#include "Chat.hpp"
#include "common/protocol/Messages.hpp"
#include "graphics/Types.hpp"

#include <gtest/gtest.h>

#include <string>

using client::ChatState;
using rtype::protocol::MaxChatLength;

TEST(Chat, DropsTheOldestLineWhenFull)
{
    ChatState chat;

    for (std::size_t index = 0; index <= client::MaxChatLines; ++index) {
        client::addChatLine(chat, 1, std::to_string(index));
    }

    ASSERT_EQ(chat.lines.size(), client::MaxChatLines);
    EXPECT_EQ(chat.lines.front().text, "1");
}

TEST(Chat, LinesExpireAfterTheirTime)
{
    ChatState chat;
    client::addChatLine(chat, 1, "hello");

    client::updateChatLines(chat, client::ChatLineTime - 0.5F);
    EXPECT_EQ(chat.lines.size(), 1U);

    client::updateChatLines(chat, 1.0F);
    EXPECT_TRUE(chat.lines.empty());
}

TEST(Chat, LinesStayWhileThePlayerTypes)
{
    ChatState chat;
    chat.open = true;
    client::addChatLine(chat, 1, "hello");

    client::updateChatLines(chat, client::ChatLineTime * 2.0F);

    EXPECT_EQ(chat.lines.size(), 1U);
}

TEST(Chat, AccentsAreRemoved)
{
    EXPECT_EQ(client::toChatCharacter(U'\u00E9'), 'e');
    EXPECT_EQ(client::toChatCharacter(U'\u00E0'), 'a');
    EXPECT_EQ(client::toChatCharacter(U'\u00E7'), 'c');
    EXPECT_EQ(client::toChatCharacter(U'\u00C9'), 'E');
    EXPECT_EQ(client::toChatCharacter(U'\u00FF'), 'y');
    EXPECT_EQ(client::toChatCharacter(U'\u0153'), 'o');
}

TEST(Chat, OnlyChatCharactersAreKept)
{
    EXPECT_EQ(client::toChatCharacter(U'a'), 'a');
    EXPECT_EQ(client::toChatCharacter(U' '), ' ');
    EXPECT_EQ(client::toChatCharacter(U'~'), '~');
    EXPECT_FALSE(client::toChatCharacter(U'\r').has_value());
    EXPECT_FALSE(client::toChatCharacter(U'\x1B').has_value());
    EXPECT_FALSE(client::toChatCharacter(U'\x7F').has_value());
    EXPECT_FALSE(client::toChatCharacter(U'\u20AC').has_value());
    EXPECT_FALSE(client::toChatCharacter(U'\U0001F600').has_value());
}

TEST(Chat, TypingFillsTheDraft)
{
    std::string draft;

    client::typeInDraft(draft, U"h\u00E9llo\r");

    EXPECT_EQ(draft, "hello");
}

TEST(Chat, BackspaceRemovesTheLastCharacter)
{
    std::string draft = "ab";
    const std::u32string typed{client::gfx::TextBackspace,
                               client::gfx::TextBackspace,
                               client::gfx::TextBackspace, U'c'};

    client::typeInDraft(draft, typed);

    EXPECT_EQ(draft, "c");
}

TEST(Chat, TheDraftStopsAtTheLongestLine)
{
    std::string draft;

    client::typeInDraft(draft, std::u32string(MaxChatLength + 5, U'a'));

    EXPECT_EQ(draft.size(), MaxChatLength);
}

TEST(Chat, ClosingGivesTheDraftWithoutItsOuterSpaces)
{
    ChatState chat;
    chat.open = true;
    chat.draft = "  hello there ";

    EXPECT_EQ(client::closeChat(chat), "hello there");
    EXPECT_FALSE(chat.open);
    EXPECT_TRUE(chat.draft.empty());
}

TEST(Chat, ClosingABlankDraftSendsNothing)
{
    ChatState chat;
    chat.open = true;
    chat.draft = "   ";

    EXPECT_FALSE(client::closeChat(chat).has_value());
    EXPECT_FALSE(chat.open);
}
