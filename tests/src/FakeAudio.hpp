/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** FakeAudio
*/

#ifndef TESTS_FAKEAUDIO_HPP
#define TESTS_FAKEAUDIO_HPP

#include "audio/IAudio.hpp"

#include <cstdint>
#include <string>
#include <vector>

// Records what the board asks. Ids count the files loaded; the listed path
// fails to load, like a missing file.
class FakeAudio final : public client::audio::IAudio {
  public:
    std::vector<std::string> paths;
    std::vector<std::uint32_t> playedSounds;
    std::vector<std::uint32_t> stoppedSounds;
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
    void stopSound(client::audio::Sound sound) override
    {
        stoppedSounds.push_back(sound.id);
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

#endif // TESTS_FAKEAUDIO_HPP
