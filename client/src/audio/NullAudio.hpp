/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NullAudio
*/

#ifndef CLIENT_AUDIO_NULLAUDIO_HPP
#define CLIENT_AUDIO_NULLAUDIO_HPP

#include "audio/IAudio.hpp"

namespace client::audio {

/**
 * @brief The silent backend: for the clients when no audio device is available.
 */
class NullAudio final : public IAudio {
  public:
    [[nodiscard]] Sound loadSound(const std::string &) override { return {}; }
    [[nodiscard]] Music loadMusic(const std::string &) override { return {}; }
    void playSound(Sound) override {}
    void stopSound(Sound) override {}
    void playMusic(Music) override {}
    void stopMusic() override {}
    void setSoundVolume(float) override {}
    void setMusicVolume(float) override {}
    void update() override {}
};

} // namespace client::audio

#endif // CLIENT_AUDIO_NULLAUDIO_HPP
