/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SoundBoard
*/

#include "audio/SoundBoard.hpp"

namespace client::audio {

namespace {

/**
 * @struct SoundFile
 * @brief The file of a sound.
 * @param event The sound.
 * @param path The file, relative to the executable.
 **/
struct SoundFile {
    SoundEvent event;
    const char *path;
};

/**
 * @struct MusicFile
 * @brief The file of a music.
 * @param track The music.
 * @param path The file, relative to the executable.
 **/
struct MusicFile {
    MusicTrack track;
    const char *path;
};

/** @brief The file of each sound, in any order. */
constexpr std::array SoundFiles = {
    SoundFile{SoundEvent::PlayerShot, "assets/sounds/player_shot.wav"},
    SoundFile{SoundEvent::ChargedShot, "assets/sounds/charged_shot.wav"},
    SoundFile{SoundEvent::EnemyShot, "assets/sounds/enemy_shot.wav"},
    SoundFile{SoundEvent::Explosion, "assets/sounds/explosion.wav"},
    SoundFile{SoundEvent::PowerUp, "assets/sounds/power_up.wav"},
    SoundFile{SoundEvent::LevelStart, "assets/sounds/level_start.wav"},
    SoundFile{SoundEvent::GameOver, "assets/sounds/game_over.wav"},
};

/** @brief The file of each music, in any order. */
constexpr std::array MusicFiles = {
    MusicFile{MusicTrack::Menu, "assets/music/menu.ogg"},
    MusicFile{MusicTrack::Game, "assets/music/game.ogg"},
};

/**
 * @brief Tells whether every sound and music has exactly one file, which
 * stops the build if one was added without its file.
 * @return True if the tables are complete.
 **/
constexpr bool everyFileIsListedOnce()
{
    for (std::size_t i = 0; i < SoundFiles.size(); ++i) {
        for (std::size_t j = i + 1; j < SoundFiles.size(); ++j) {
            if (SoundFiles.at(i).event == SoundFiles.at(j).event) {
                return false;
            }
        }
    }
    for (std::size_t i = 0; i < MusicFiles.size(); ++i) {
        for (std::size_t j = i + 1; j < MusicFiles.size(); ++j) {
            if (MusicFiles.at(i).track == MusicFiles.at(j).track) {
                return false;
            }
        }
    }
    return SoundFiles.size() == static_cast<std::size_t>(SoundEvent::Count) &&
           MusicFiles.size() == static_cast<std::size_t>(MusicTrack::Count);
}

static_assert(everyFileIsListedOnce(),
              "each SoundEvent and MusicTrack needs exactly one file");

} // namespace

SoundBoard::SoundBoard(IAudio &device) : m_device(&device)
{
    for (const SoundFile &file : SoundFiles) {
        m_sounds.at(static_cast<std::size_t>(file.event)) =
            m_device->loadSound(file.path);
    }
    for (const MusicFile &file : MusicFiles) {
        m_musics.at(static_cast<std::size_t>(file.track)) =
            m_device->loadMusic(file.path);
    }
}

void SoundBoard::play(SoundEvent event)
{
    m_device->playSound(m_sounds.at(static_cast<std::size_t>(event)));
}

void SoundBoard::setMusic(MusicTrack track)
{
    if (m_track != track) {
        m_device->playMusic(m_musics.at(static_cast<std::size_t>(track)));
        m_track = track;
    }
}

void SoundBoard::update() { m_device->update(); }

} // namespace client::audio
