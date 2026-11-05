/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SoundEvent
*/

#ifndef CLIENT_AUDIO_SOUNDEVENT_HPP
#define CLIENT_AUDIO_SOUNDEVENT_HPP

#include <cstdint>

namespace client::audio {

/**
 * @enum SoundEvent
 * @brief A sound of the game. Its file is set in SoundBoard.cpp.
 * @param PlayerShot A player fired a missile.
 * @param ChargedShot A player fired a charged shot.
 * @param EnemyShot An enemy fired a missile.
 * @param Explosion A ship or an enemy exploded.
 * @param PowerUp A player took a power-up.
 * @param LevelStart A new level is announced.
 * @param GameOver The game is over.
 * @param Count Number of sounds, not a sound.
 **/
enum class SoundEvent : std::uint8_t {
    PlayerShot,
    ChargedShot,
    EnemyShot,
    Explosion,
    PowerUp,
    LevelStart,
    GameOver,
    Count,
};

/**
 * @enum MusicTrack
 * @brief A music of the game. Its file is set in SoundBoard.cpp.
 * @param Menu Outside a running game.
 * @param Game During a game.
 * @param Count Number of musics, not a music.
 **/
enum class MusicTrack : std::uint8_t {
    Menu,
    Game,
    Count,
};

} // namespace client::audio

#endif // CLIENT_AUDIO_SOUNDEVENT_HPP
