/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** settings_test
*/

#include "Settings.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

using client::readSettings;
using client::Settings;
using client::writeSettings;

namespace {

// Reads a whole file; returns its errors and leaves the result in settings.
std::vector<std::string> read(const std::string &text, Settings &settings)
{
    std::istringstream in(text);
    return readSettings(in, settings);
}

} // namespace

TEST(Settings, ReadsNothingFromAnEmptyFile)
{
    Settings settings;

    EXPECT_TRUE(read("", settings).empty());

    const Settings defaults;
    EXPECT_FLOAT_EQ(settings.musicVolume, defaults.musicVolume);
    EXPECT_FLOAT_EQ(settings.soundVolume, defaults.soundVolume);
    EXPECT_EQ(settings.autoFire, defaults.autoFire);
    EXPECT_EQ(settings.playerLabels, defaults.playerLabels);
}

TEST(Settings, ShowsThePlayerLabelsByDefault)
{
    const Settings settings;

    EXPECT_TRUE(settings.playerLabels);
    EXPECT_FALSE(settings.autoFire);
}

TEST(Settings, ReadsEverySetting)
{
    Settings settings;

    const auto errors = read("music_volume = 0.25\n"
                             "sound_volume = 0.75\n"
                             "auto_fire = on\n"
                             "player_labels = off\n",
                             settings);

    EXPECT_TRUE(errors.empty());
    EXPECT_FLOAT_EQ(settings.musicVolume, 0.25F);
    EXPECT_FLOAT_EQ(settings.soundVolume, 0.75F);
    EXPECT_TRUE(settings.autoFire);
    EXPECT_FALSE(settings.playerLabels);
}

TEST(Settings, IgnoresCommentsEmptyLinesAndBlanks)
{
    Settings settings;

    const auto errors = read("# a comment\n"
                             "\n"
                             "   \n"
                             "  music_volume=0.5  \r\n",
                             settings);

    EXPECT_TRUE(errors.empty());
    EXPECT_FLOAT_EQ(settings.musicVolume, 0.5F);
}

TEST(Settings, ReportsBadLinesWithTheirNumberAndKeepsTheOthers)
{
    Settings settings;

    const auto errors = read("no equal sign\n"
                             "\n"
                             "difficulty = hard\n"
                             "sound_volume = 0.5\n",
                             settings);

    ASSERT_EQ(errors.size(), 2U);
    EXPECT_EQ(errors.at(0), "line 1: expected name = value");
    EXPECT_EQ(errors.at(1), "line 3: unknown setting \"difficulty\"");
    EXPECT_FLOAT_EQ(settings.soundVolume, 0.5F);
}

TEST(Settings, RefusesAVolumeOutOfRange)
{
    for (const std::string value :
         {"-0.1", "1.5", "nan", "inf", "loud", "0.5x", ""}) {
        Settings settings;
        settings.musicVolume = 0.3F;

        const auto errors = read("music_volume = " + value, settings);

        EXPECT_EQ(errors.size(), 1U) << value;
        EXPECT_FLOAT_EQ(settings.musicVolume, 0.3F) << value;
    }
}

TEST(Settings, AcceptsTheBoundsOfAVolume)
{
    Settings settings;

    EXPECT_TRUE(read("music_volume = 0\nsound_volume = 1", settings).empty());

    EXPECT_FLOAT_EQ(settings.musicVolume, 0.0F);
    EXPECT_FLOAT_EQ(settings.soundVolume, 1.0F);
}

TEST(Settings, RefusesABadToggle)
{
    Settings settings;
    settings.autoFire = true;

    const auto errors = read("auto_fire = maybe", settings);

    EXPECT_EQ(errors.size(), 1U);
    EXPECT_TRUE(settings.autoFire);
}

TEST(Settings, ReadsBackWhatItWrites)
{
    Settings written;
    written.musicVolume = 0.45F;
    written.soundVolume = 0.05F;
    written.autoFire = true;
    written.playerLabels = false;
    std::ostringstream out;
    writeSettings(out, written);

    Settings settings;
    EXPECT_TRUE(read(out.str(), settings).empty());

    EXPECT_FLOAT_EQ(settings.musicVolume, written.musicVolume);
    EXPECT_FLOAT_EQ(settings.soundVolume, written.soundVolume);
    EXPECT_EQ(settings.autoFire, written.autoFire);
    EXPECT_EQ(settings.playerLabels, written.playerLabels);
}

TEST(Settings, WritesEveryToggleOfTheTable)
{
    std::ostringstream out;

    writeSettings(out, Settings{});

    for (const client::ToggleDef &toggle : client::Toggles) {
        EXPECT_NE(out.str().find(toggle.key), std::string::npos) << toggle.key;
    }
}

TEST(Settings, LoadsTheDefaultsWithoutAFile)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "rtype_no_settings.cfg";
    std::filesystem::remove(path);

    const Settings settings = client::loadSettingsFile(path.string());

    EXPECT_FLOAT_EQ(settings.musicVolume, Settings{}.musicVolume);
}

TEST(Settings, LoadsWhatItSaves)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "rtype_settings_test.cfg";
    Settings written;
    written.musicVolume = 0.15F;
    written.autoFire = true;

    EXPECT_TRUE(client::saveSettingsFile(path.string(), written));
    const Settings settings = client::loadSettingsFile(path.string());
    std::filesystem::remove(path);

    EXPECT_FLOAT_EQ(settings.musicVolume, written.musicVolume);
    EXPECT_TRUE(settings.autoFire);
}

TEST(Settings, ReportsAFileItCannotWrite)
{
    const std::filesystem::path path = std::filesystem::temp_directory_path() /
                                       "rtype_no_such_folder" / "settings.cfg";

    EXPECT_FALSE(client::saveSettingsFile(path.string(), Settings{}));
}
