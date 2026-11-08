/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** controls_test
*/

#include "Controls.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using client::Control;
using client::Controls;
using client::PlayerControls;
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
        if (key == Key::Escape) {
            continue;
        }
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
    PlayerControls players;
    const Controls &controls = players.keyboard;
    std::istringstream file{"# ZQSD layout\n"
                            "\n"
                            "  UP = z  Up\n"
                            "left=Q\n"
                            "fire = Enter Enter\r\n"};
    EXPECT_TRUE(client::readControls(file, players).empty());
    EXPECT_EQ(keysOf(controls, Control::Up),
              (std::vector<Key>{Key::Z, Key::Up}));
    EXPECT_EQ(keysOf(controls, Control::Left), (std::vector<Key>{Key::Q}));
    EXPECT_EQ(keysOf(controls, Control::Fire), (std::vector<Key>{Key::Enter}));
    EXPECT_EQ(keysOf(controls, Control::Down),
              keysOf(client::defaultControls(), Control::Down));
}

TEST(Controls, BadLinesAreReportedAndSkipped)
{
    PlayerControls players;
    const Controls &controls = players.keyboard;
    std::istringstream file{"jump = Space\n"
                            "fire Space\n"
                            "fire = Banana\n"
                            "down =\n"
                            "up = A B C D E F G H I\n"
                            "right = L\n"};
    const std::vector<std::string> errors = client::readControls(file, players);
    ASSERT_EQ(errors.size(), 5U);
    EXPECT_EQ(errors[0].rfind("line 1:", 0), 0U);
    EXPECT_EQ(errors[4].rfind("line 5:", 0), 0U);
    EXPECT_EQ(controls.at(static_cast<std::size_t>(Control::Fire)),
              keysOf(client::defaultControls(), Control::Fire));
    EXPECT_EQ(keysOf(controls, Control::Right), (std::vector<Key>{Key::L}));
}

TEST(Controls, ReadsBackWhatItWrites)
{
    PlayerControls written;
    written.keyboard.at(static_cast<std::size_t>(Control::Fire)) = {Key::Enter,
                                                                    Key::G};
    written.gamepad.at(static_cast<std::size_t>(Control::Fire)) = {Key::PadB};
    std::ostringstream out;
    client::writeControls(out, written);

    PlayerControls controls;
    std::istringstream in(out.str());
    EXPECT_TRUE(client::readControls(in, controls).empty());

    EXPECT_EQ(controls, written);
}

TEST(Controls, WritesTheNamesOfTheFile)
{
    std::ostringstream out;

    client::writeControls(out, PlayerControls{});

    EXPECT_NE(out.str().find("fire = Space RightCtrl\n"), std::string::npos);
    EXPECT_NE(out.str().find("autofire = F RightShift\n"), std::string::npos);
    EXPECT_NE(out.str().find("pad_fire = PadA PadRT\n"), std::string::npos);
    EXPECT_EQ(out.str().find("pad_chat"), std::string::npos);
}

TEST(Controls, SkipsAControlWithoutKeysWhenWriting)
{
    PlayerControls controls;
    controls.keyboard.at(static_cast<std::size_t>(Control::Fire)).clear();
    std::ostringstream out;

    client::writeControls(out, controls);

    EXPECT_EQ(out.str().find("fire ="), out.str().find("autofire =") + 4);
}

TEST(Controls, SavesAFileItCanRead)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "rtype_controls_test.cfg";

    EXPECT_TRUE(client::saveControlsFile(path.string(), PlayerControls{}));
    EXPECT_TRUE(std::filesystem::exists(path));
    std::filesystem::remove(path);
}

TEST(Controls, ReportsAFileItCannotWrite)
{
    const std::filesystem::path path = std::filesystem::temp_directory_path() /
                                       "rtype_no_such_folder" / "controls.cfg";

    EXPECT_FALSE(client::saveControlsFile(path.string(), PlayerControls{}));
}

TEST(Controls, LoadsTheFileGivenOverTheDefaults)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "rtype_controls_load_test.cfg";
    std::ofstream(path) << "fire = G\n";

    const PlayerControls controls = client::loadControlsFile(path.string());
    std::filesystem::remove(path);

    EXPECT_EQ(keysOf(controls.keyboard, Control::Fire),
              (std::vector<Key>{Key::G}));
    EXPECT_EQ(keysOf(controls.keyboard, Control::Up),
              keysOf(client::defaultControls(), Control::Up));
    EXPECT_EQ(controls.gamepad, client::defaultPadControls());
}

TEST(Controls, KeepsTheDefaultsOfABadLineWhenLoading)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "rtype_controls_bad_test.cfg";
    std::ofstream(path) << "fire = NotAKey\n";

    const PlayerControls controls = client::loadControlsFile(path.string());
    std::filesystem::remove(path);

    EXPECT_EQ(controls, PlayerControls{});
}

TEST(Controls, RefusesAFileGivenThatCannotBeOpened)
{
    const std::filesystem::path path = std::filesystem::temp_directory_path() /
                                       "rtype_no_such_folder" / "controls.cfg";

    EXPECT_THROW((void)client::loadControlsFile(path.string()),
                 std::runtime_error);
}

TEST(Controls, LoadsTheDefaultFileOnlyIfItExists)
{
    const std::filesystem::path previous = std::filesystem::current_path();
    const std::filesystem::path folder =
        std::filesystem::temp_directory_path() / "rtype_controls_default_test";
    std::filesystem::create_directories(folder);
    std::filesystem::current_path(folder);

    const PlayerControls withoutFile = client::loadControlsFile(std::nullopt);
    std::ofstream(client::DefaultControlsPath) << "fire = G\n";
    const PlayerControls withFile = client::loadControlsFile(std::nullopt);

    std::filesystem::current_path(previous);
    std::filesystem::remove_all(folder);
    EXPECT_EQ(withoutFile, PlayerControls{});
    EXPECT_EQ(keysOf(withFile.keyboard, Control::Fire),
              (std::vector<Key>{Key::G}));
}

TEST(Controls, BindKeyReplacesOnePlace)
{
    Controls controls = client::defaultControls();

    client::bindKey(controls, Control::Fire, 1, Key::G);

    EXPECT_EQ(controls.at(static_cast<std::size_t>(Control::Fire)),
              (std::vector<Key>{Key::Space, Key::G}));
}

TEST(Controls, BindKeyAddsPastTheLastPlace)
{
    Controls controls = client::defaultControls();
    controls.at(static_cast<std::size_t>(Control::Fire)) = {Key::Space};

    client::bindKey(controls, Control::Fire, 1, Key::G);

    EXPECT_EQ(controls.at(static_cast<std::size_t>(Control::Fire)),
              (std::vector<Key>{Key::Space, Key::G}));
}

TEST(Controls, BindKeySwapsTheTwoKeysOfAControl)
{
    Controls controls = client::defaultControls();

    client::bindKey(controls, Control::Fire, 0, Key::RightControl);

    EXPECT_EQ(controls.at(static_cast<std::size_t>(Control::Fire)),
              (std::vector<Key>{Key::RightControl, Key::Space}));
}

TEST(Controls, BindKeyDoesNotAddAKeyTwice)
{
    Controls controls = client::defaultControls();
    controls.at(static_cast<std::size_t>(Control::Fire)) = {Key::Space};

    client::bindKey(controls, Control::Fire, 1, Key::Space);

    EXPECT_EQ(controls.at(static_cast<std::size_t>(Control::Fire)),
              std::vector<Key>{Key::Space});
}

TEST(Controls, FindsAnotherControlWithAKey)
{
    const Controls controls = client::defaultControls();

    EXPECT_EQ(client::otherControlWith(controls, Control::Fire, Key::F),
              Control::AutoFire);
    EXPECT_FALSE(client::otherControlWith(controls, Control::Fire, Key::Space)
                     .has_value());
    EXPECT_FALSE(
        client::otherControlWith(controls, Control::Fire, Key::G).has_value());
}

TEST(Controls, GamepadMovesWithTheDPadOrTheStick)
{
    const Controls pad = client::defaultPadControls();

    EXPECT_EQ(keysOf(pad, Control::Up),
              (std::vector<Key>{Key::PadUp, Key::PadStickUp}));
    EXPECT_EQ(keysOf(pad, Control::Fire),
              (std::vector<Key>{Key::PadA, Key::PadRT}));
    EXPECT_EQ(keysOf(pad, Control::AutoFire), (std::vector<Key>{Key::PadRB}));
    EXPECT_TRUE(keysOf(pad, Control::Chat).empty());
}

TEST(Controls, DefaultGamepadButtonsAreOnlyGamepadKeys)
{
    for (const auto &keys : client::defaultPadControls()) {
        for (const Key key : keys) {
            EXPECT_TRUE(client::gfx::isPadKey(key)) << client::keyName(key);
        }
    }
    for (const auto &keys : client::defaultControls()) {
        for (const Key key : keys) {
            EXPECT_FALSE(client::gfx::isPadKey(key)) << client::keyName(key);
        }
    }
}

TEST(Controls, EachDeviceHasItsOwnLines)
{
    PlayerControls controls;
    std::istringstream file{"pad_fire = PadB\n"
                            "PAD_Up = PadUp\n"
                            "left = Q\n"};

    EXPECT_TRUE(client::readControls(file, controls).empty());

    EXPECT_EQ(keysOf(controls.gamepad, Control::Fire),
              (std::vector<Key>{Key::PadB}));
    EXPECT_EQ(keysOf(controls.gamepad, Control::Up),
              (std::vector<Key>{Key::PadUp}));
    EXPECT_EQ(keysOf(controls.keyboard, Control::Fire),
              keysOf(client::defaultControls(), Control::Fire));
    EXPECT_EQ(keysOf(controls.keyboard, Control::Left),
              (std::vector<Key>{Key::Q}));
    EXPECT_EQ(keysOf(controls.gamepad, Control::Left),
              keysOf(client::defaultPadControls(), Control::Left));
}

TEST(Controls, RefusesAKeyOfTheOtherDevice)
{
    PlayerControls controls;
    std::istringstream file{"fire = Space PadA\n"
                            "pad_fire = Space\n"
                            "pad_ = PadA\n"
                            "pad_jump = PadA\n"};

    const std::vector<std::string> errors =
        client::readControls(file, controls);

    ASSERT_EQ(errors.size(), 4U);
    EXPECT_NE(errors[0].find("pad_fire"), std::string::npos);
    EXPECT_EQ(controls, PlayerControls{});
}
