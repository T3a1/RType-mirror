/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SfmlAudio
*/

#ifndef CLIENT_AUDIO_SFMLAUDIO_HPP
#define CLIENT_AUDIO_SFMLAUDIO_HPP

#include "../../graphics/sfml/ExecutableDirectory.hpp"
#include "../NullAudio.hpp"
#include "engine/Logger.hpp"

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <utility>

namespace client::audio {

/** @brief IAudio backend using SFML. */
class SfmlAudio final : public IAudio {
  public:
    [[nodiscard]] Sound loadSound(const std::string &path) override;
    [[nodiscard]] Music loadMusic(const std::string &path) override;
    void playSound(Sound sound) override;
    void stopSound(Sound sound) override;
    void playMusic(Music music) override;
    void stopMusic() override;
    void setSoundVolume(float volume) override;
    void setMusicVolume(float volume) override;
    void update() override;

  private:
    /** @brief The sound plays from the buffer, declared first so it outlives
     * the sound. */
    struct LoadedSound {
        explicit LoadedSound(const sf::SoundBuffer &loaded) : buffer(loaded) {}

        sf::SoundBuffer buffer;
        sf::Sound sound{buffer};
    };

    std::filesystem::path m_directory = gfx::executableDirectory();
    std::unordered_map<std::uint32_t, std::unique_ptr<LoadedSound>> m_sounds;
    std::unordered_map<std::uint32_t, std::unique_ptr<sf::Music>> m_musics;
    std::uint32_t m_nextId = 1;
    std::uint32_t m_currentMusic = 0;
    float m_soundVolume = 100.0F;
    float m_musicVolume = 100.0F;
};

} // namespace client::audio

#endif // CLIENT_AUDIO_SFMLAUDIO_HPP
