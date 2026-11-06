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
 * @param Charge The local player is charging a shot.
 * @param ChargedShot A player fired a charged shot.
 * @param EnemyShot An enemy fired a missile.
 * @param EnemyHit An enemy lost health without being destroyed yet.
 * @param Explosion A small enemy exploded.
 * @param ExplosionLarge A big enemy or another player's ship exploded.
 * @param ExplosionBoss The boss exploded.
 * @param PlayerDeath The local player's ship exploded.
 * @param BossAlert The boss arrived.
 * @param PowerUpSpawn A power-up appeared.
 * @param PowerUp A player took a power-up.
 * @param GameStart The game of the room started.
 * @param LevelStart A new level is announced.
 * @param GameOver The game is over.
 * @param PlayerJoined Another player joined the room.
 * @param PlayerLeft A player left the room.
 * @param MenuMove The selection of a menu moved.
 * @param MenuConfirm An item of a menu was chosen.
 * @param MenuBack The player went back or left a screen.
 * @param MenuRefused The server refused what the player asked.
 * @param Count Number of sounds, not a sound.
 **/
enum class SoundEvent : std::uint8_t {
    PlayerShot,
    Charge,
    ChargedShot,
    EnemyShot,
    EnemyHit,
    Explosion,
    ExplosionLarge,
    ExplosionBoss,
    PlayerDeath,
    BossAlert,
    PowerUpSpawn,
    PowerUp,
    GameStart,
    LevelStart,
    GameOver,
    PlayerJoined,
    PlayerLeft,
    MenuMove,
    MenuConfirm,
    MenuBack,
    MenuRefused,
    Count,
};

/**
 * @enum MusicTrack
 * @brief A music of the game. Its file is set in SoundBoard.cpp.
 * @param Menu Outside a running game.
 * @param Game During a game.
 * @param Boss While a boss is alive.
 * @param Count Number of musics, not a music.
 **/
enum class MusicTrack : std::uint8_t {
    Menu,
    Game,
    Boss,
    Count,
};

} // namespace client::audio

#endif // CLIENT_AUDIO_SOUNDEVENT_HPP
