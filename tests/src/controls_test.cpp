/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** controls_test
*/

#include "Controls.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <vector>

using client::Control;
using client::Controls;
using client::gfx::Key;

namespace {

const std::vector<Key> &keysOf(const Controls &controls, Control control)
{
    return controls.at(static_cast<std::size_t>(control));
}

} // namespace

TEST(Controls, DefaultsWorkWithEitherHand)
{
    const Controls controls = client::defaultControls();
    for (const auto &keys : controls) {
        EXPECT_FALSE(keys.empty());
    }
    EXPECT_EQ(keysOf(controls, Control::Up),
              (std::vector<Key>{Key::Up, Key::W}));
    EXPECT_EQ(keysOf(controls, Control::Fire),
              (std::vector<Key>{Key::Space, Key::RightControl}));
}

TEST(Controls, KeyNamesRoundTrip)
{
    for (std::size_t i = 0; i < client::gfx::KeyCount; ++i) {
        const auto key = static_cast<Key>(i);
        EXPECT_EQ(client::keyFromName(client::keyName(key)), key)
            << client::keyName(key);
    }
}

TEST(Controls, KeyNamesIgnoreCase)
{
    EXPECT_EQ(client::keyFromName("space"), Key::Space);
    EXPECT_EQ(client::keyFromName("z"), Key::Z);
    EXPECT_EQ(client::keyFromName("7"), Key::Num7);
    EXPECT_FALSE(client::keyFromName("Escape").has_value());
    EXPECT_FALSE(client::keyFromName("").has_value());
}

TEST(Controls, FileReplacesTheKeysOfEachLine)
{
    Controls controls = client::defaultControls();
    std::istringstream file{"# ZQSD layout\n"
                            "\n"
                            "  UP = z  Up\n"
                            "left=Q\n"
                            "fire = Enter Enter\r\n"};
    EXPECT_TRUE(client::readControls(file, controls).empty());
    EXPECT_EQ(keysOf(controls, Control::Up),
              (std::vector<Key>{Key::Z, Key::Up}));
    EXPECT_EQ(keysOf(controls, Control::Left), (std::vector<Key>{Key::Q}));
    EXPECT_EQ(keysOf(controls, Control::Fire), (std::vector<Key>{Key::Enter}));
    EXPECT_EQ(keysOf(controls, Control::Down),
              keysOf(client::defaultControls(), Control::Down));
}

TEST(Controls, BadLinesAreReportedAndSkipped)
{
    Controls controls = client::defaultControls();
    std::istringstream file{"jump = Space\n"
                            "fire Space\n"
                            "fire = Banana\n"
                            "down =\n"
                            "up = A B C D E F G H I\n"
                            "right = L\n"};
    const std::vector<std::string> errors =
        client::readControls(file, controls);
    ASSERT_EQ(errors.size(), 5U);
    EXPECT_EQ(errors[0].rfind("line 1:", 0), 0U);
    EXPECT_EQ(errors[4].rfind("line 5:", 0), 0U);
    EXPECT_EQ(controls.at(static_cast<std::size_t>(Control::Fire)),
              keysOf(client::defaultControls(), Control::Fire));
    EXPECT_EQ(keysOf(controls, Control::Right), (std::vector<Key>{Key::L}));
}
