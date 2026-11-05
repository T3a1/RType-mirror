/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** player_notices_test
*/

#include "PlayerNotices.hpp"

#include <gtest/gtest.h>

using client::addPlayerNotice;
using client::PlayerNotice;
using client::updatePlayerNotices;

TEST(PlayerNotices, AddsTheNewestLast)
{
    std::vector<PlayerNotice> notices;

    addPlayerNotice(notices, 2, true);
    addPlayerNotice(notices, 3, false);

    ASSERT_EQ(notices.size(), 2U);
    EXPECT_EQ(notices.front().slot, 2);
    EXPECT_TRUE(notices.front().joined);
    EXPECT_EQ(notices.back().slot, 3);
    EXPECT_FALSE(notices.back().joined);
}

TEST(PlayerNotices, DropsTheOldestWhenFull)
{
    std::vector<PlayerNotice> notices;

    for (std::size_t index = 0; index <= client::MaxPlayerNotices; ++index) {
        addPlayerNotice(notices, static_cast<std::uint8_t>(index), false);
    }

    ASSERT_EQ(notices.size(), client::MaxPlayerNotices);
    EXPECT_EQ(notices.front().slot, 1);
}

TEST(PlayerNotices, ExpireAfterTheirTime)
{
    std::vector<PlayerNotice> notices;
    addPlayerNotice(notices, 1, false);

    updatePlayerNotices(notices, client::PlayerNoticeTime - 0.5F);
    EXPECT_EQ(notices.size(), 1U);

    updatePlayerNotices(notices, 1.0F);
    EXPECT_TRUE(notices.empty());
}
