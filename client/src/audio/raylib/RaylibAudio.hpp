/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** RaylibAudio
*/

#ifndef CLIENT_AUDIO_RAYLIBAUDIO_HPP
#define CLIENT_AUDIO_RAYLIBAUDIO_HPP

#include "../NullAudio.hpp"
#include "engine/Logger.hpp"

#include <raylib.h>

#include <algorithm>
#include <cstdint>
#include <unordered_map>

namespace client::audio {

/**
 * @brief IAudio backend using Raylib. It closes the audio device opened by
 * createAudio(), so only one may exist at a time.
 */
class RaylibAudio final : public IAudio {
  public:
    RaylibAudio() = default;
    ~RaylibAudio() override;

    RaylibAudio(const RaylibAudio &) = delete;
    RaylibAudio &operator=(const RaylibAudio &) = delete;
    RaylibAudio(RaylibAudio &&) = delete;
    RaylibAudio &operator=(RaylibAudio &&) = delete;

    [[nodiscard]] Sound loadSound(const std::string &path) override;
    [[nodiscard]] Music loadMusic(const std::string &path) override;
    void playSound(Sound sound) override;
    void playMusic(Music music) override;
    void stopMusic() override;
    void setSoundVolume(float volume) override;
    void setMusicVolume(float volume) override;
    void update() override;

  private:
    std::unordered_map<std::uint32_t, ::Sound> m_sounds;
    std::unordered_map<std::uint32_t, ::Music> m_musics;
    std::uint32_t m_nextId = 1;
    std::uint32_t m_currentMusic = 0;
    float m_soundVolume = 1.0F;
    float m_musicVolume = 1.0F;
};

} // namespace client::audio

#endif // CLIENT_AUDIO_RAYLIBAUDIO_HPP
