/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** sound_board_test
*/

#include "audio/SoundBoard.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

namespace {

using client::audio::MusicTrack;
using client::audio::SoundBoard;
using client::audio::SoundEvent;

// Records what the board asks. Ids count the files loaded; the listed path
// fails to load, like a missing file.
class FakeAudio final : public client::audio::IAudio {
  public:
    std::vector<std::string> paths;
    std::vector<std::uint32_t> playedSounds;
    std::vector<std::uint32_t> playedMusics;
    std::string missing;
    int updates = 0;

    [[nodiscard]] client::audio::Sound
    loadSound(const std::string &path) override
    {
        paths.push_back(path);
        return {.id = path == missing
                          ? 0
                          : static_cast<std::uint32_t>(paths.size())};
    }
    [[nodiscard]] client::audio::Music
    loadMusic(const std::string &path) override
    {
        paths.push_back(path);
        return {.id = static_cast<std::uint32_t>(paths.size())};
    }
    void playSound(client::audio::Sound sound) override
    {
        playedSounds.push_back(sound.id);
    }
    void playMusic(client::audio::Music music) override
    {
        playedMusics.push_back(music.id);
    }
    void stopMusic() override {}
    void setSoundVolume(float /*volume*/) override {}
    void setMusicVolume(float /*volume*/) override {}
    void update() override { ++updates; }
};

} // namespace

TEST(SoundBoard, LoadsOneFilePerSoundAndMusic)
{
    FakeAudio audio;
    const SoundBoard board{audio};

    EXPECT_EQ(audio.paths.size(),
              static_cast<std::size_t>(SoundEvent::Count) +
                  static_cast<std::size_t>(MusicTrack::Count));
}

TEST(SoundBoard, PlaysTheFileOfEachSound)
{
    FakeAudio audio;
    SoundBoard board{audio};

    board.play(SoundEvent::Explosion);
    board.play(SoundEvent::PlayerShot);

    ASSERT_EQ(audio.playedSounds.size(), 2U);
    EXPECT_EQ(audio.paths.at(audio.playedSounds.at(0) - 1),
              "assets/sounds/explosion.wav");
    EXPECT_EQ(audio.paths.at(audio.playedSounds.at(1) - 1),
              "assets/sounds/player_shot.wav");
}

TEST(SoundBoard, PlaysAMissingSoundWithIdZero)
{
    FakeAudio audio;
    audio.missing = "assets/sounds/explosion.wav";
    SoundBoard board{audio};

    board.play(SoundEvent::Explosion);

    EXPECT_EQ(audio.playedSounds, (std::vector<std::uint32_t>{0}));
}

TEST(SoundBoard, RestartsAMusicOnlyWhenTheTrackChanges)
{
    FakeAudio audio;
    SoundBoard board{audio};

    board.setMusic(MusicTrack::Menu);
    board.setMusic(MusicTrack::Menu);
    EXPECT_EQ(audio.playedMusics.size(), 1U);

    board.setMusic(MusicTrack::Game);
    board.setMusic(MusicTrack::Game);
    ASSERT_EQ(audio.playedMusics.size(), 2U);
    EXPECT_EQ(audio.paths.at(audio.playedMusics.at(0) - 1),
              "assets/music/menu.ogg");
    EXPECT_EQ(audio.paths.at(audio.playedMusics.at(1) - 1),
              "assets/music/game.ogg");
}

TEST(SoundBoard, UpdateReachesTheBackend)
{
    FakeAudio audio;
    SoundBoard board{audio};

    board.update();

    EXPECT_EQ(audio.updates, 1);
}
