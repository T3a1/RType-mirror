/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SoundBoard
*/

#ifndef CLIENT_AUDIO_SOUNDBOARD_HPP
#define CLIENT_AUDIO_SOUNDBOARD_HPP

#include "audio/IAudio.hpp"
#include "audio/SoundEvent.hpp"

#include <array>
#include <cstddef>
#include <optional>

namespace client::audio {

/**
 * @class SoundBoard
 * @brief The sounds and musics of the game, played by name. A missing file
 * stays silent.
 **/
class SoundBoard {
  public:
    /**
     * @brief Loads every sound and music.
     * @param device The audio backend, which must outlive the board.
     **/
    explicit SoundBoard(IAudio &device);

    /**
     * @brief Plays a sound at once. If it is already playing, it restarts.
     * @param event The sound.
     **/
    void play(SoundEvent event);

    /**
     * @brief Stops a sound, does nothing if it is not playing.
     * @param event The sound.
     **/
    void stop(SoundEvent event);

    /**
     * @brief Loops a music, does nothing if it is already playing.
     * @param track The music.
     **/
    void setMusic(MusicTrack track);

    /**
     * @brief Keeps the music playing: call it once per frame.
     **/
    void update();

  private:
    IAudio *m_device;
    std::array<Sound, static_cast<std::size_t>(SoundEvent::Count)> m_sounds;
    std::array<Music, static_cast<std::size_t>(MusicTrack::Count)> m_musics;
    std::optional<MusicTrack> m_track;
};

} // namespace client::audio

#endif // CLIENT_AUDIO_SOUNDBOARD_HPP
