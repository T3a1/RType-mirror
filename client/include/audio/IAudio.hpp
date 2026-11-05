/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** IAudio
*/

#ifndef CLIENT_AUDIO_IAUDIO_HPP
#define CLIENT_AUDIO_IAUDIO_HPP

#include <cstdint>
#include <memory>
#include <string>

namespace client::audio {

/** @brief A loaded sound effect, id 0 if loading failed. */
struct Sound {
    std::uint32_t id = 0;
};

/** @brief A loaded music track, id 0 if loading failed. */
struct Music {
    std::uint32_t id = 0;
};

/**
 * @brief Sound effects and music, independent of the audio library.
 *
 * The client only talks to this interface. Each backend implements it in
 * client/src/audio/<backend>/, the only place that includes the library's
 * headers. Nothing here throws: a missing file or audio device only means
 * silence.
 */
class IAudio {
  public:
    IAudio() = default;
    virtual ~IAudio() = default;

    IAudio(const IAudio &) = delete;
    IAudio &operator=(const IAudio &) = delete;
    IAudio(IAudio &&) = delete;
    IAudio &operator=(IAudio &&) = delete;

    /**
     * @brief Loads a sound effect.
     * @param path Path relative to the directory of the executable.
     * @return The sound, with id 0 if loading failed.
     */
    [[nodiscard]] virtual Sound loadSound(const std::string &path) = 0;
    /**
     * @brief Opens a music track, streamed from its file.
     * @param path Path relative to the directory of the executable.
     * @return The music, with id 0 if loading failed.
     */
    [[nodiscard]] virtual Music loadMusic(const std::string &path) = 0;

    /** @brief Plays a sound once, does nothing for id 0. */
    virtual void playSound(Sound sound) = 0;
    /** @brief Stops a sound, does nothing for id 0 or if it is not playing. */
    virtual void stopSound(Sound sound) = 0;
    /** @brief Loops a music instead of the current one, does nothing for
     * id 0. */
    virtual void playMusic(Music music) = 0;
    virtual void stopMusic() = 0;

    /** @brief Sets the volume of the sound effects, from 0 to 1. */
    virtual void setSoundVolume(float volume) = 0;
    /** @brief Sets the volume of the music, from 0 to 1. */
    virtual void setMusicVolume(float volume) = 0;

    /** @brief Keeps the music playing: call it once per frame. */
    virtual void update() = 0;
};

/**
 * @brief Opens the audio device with the backend chosen at build time.
 *
 * Defined by the backend's source file.
 * @return The backend, or a silent one if there is no audio device.
 */
[[nodiscard]] std::unique_ptr<IAudio> createAudio();

} // namespace client::audio

#endif // CLIENT_AUDIO_IAUDIO_HPP
