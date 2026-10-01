/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** sanity_test
*/

#include <gtest/gtest.h>

#include <string>

TEST(Sanity, BasicAssertions)
{
    EXPECT_EQ(2 + 2, 4);
    EXPECT_STRNE("server", "client");
}

TEST(Sanity, StringConcatenation)
{
    const std::string name = std::string("r-") + "type";

    ASSERT_EQ(name.size(), 6U);
    EXPECT_EQ(name, "r-type");
}
