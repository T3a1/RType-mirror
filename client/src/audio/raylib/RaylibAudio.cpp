/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** RaylibAudio
*/

#include "RaylibAudio.hpp"

namespace client::audio {

std::unique_ptr<IAudio> createAudio()
{
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        BB_WARN("No audio device: the game runs without sound");
        return std::make_unique<NullAudio>();
    }
    return std::make_unique<RaylibAudio>();
}

RaylibAudio::~RaylibAudio()
{
    for (const auto &[id, sound] : m_sounds) {
        UnloadSound(sound);
    }
    for (const auto &[id, music] : m_musics) {
        UnloadMusicStream(music);
    }
    CloseAudioDevice();
}

Sound RaylibAudio::loadSound(const std::string &path)
{
    const std::string fullPath = std::string{GetApplicationDirectory()} + path;
    const ::Sound sound =
        FileExists(fullPath.c_str()) ? LoadSound(fullPath.c_str()) : ::Sound{};
    if (!IsSoundValid(sound)) {
        BB_WARN("Cannot load the sound " << path);
        return {};
    }
    SetSoundVolume(sound, m_soundVolume);
    const std::uint32_t id = m_nextId++;
    m_sounds.emplace(id, sound);
    return {.id = id};
}

Music RaylibAudio::loadMusic(const std::string &path)
{
    const std::string fullPath = std::string{GetApplicationDirectory()} + path;
    const ::Music music = FileExists(fullPath.c_str())
                              ? LoadMusicStream(fullPath.c_str())
                              : ::Music{};
    if (!IsMusicValid(music)) {
        BB_WARN("Cannot load the music " << path);
        return {};
    }
    SetMusicVolume(music, m_musicVolume);
    const std::uint32_t id = m_nextId++;
    m_musics.emplace(id, music);
    return {.id = id};
}

void RaylibAudio::playSound(Sound sound)
{
    const auto it = m_sounds.find(sound.id);
    if (it != m_sounds.end()) {
        PlaySound(it->second);
    }
}

void RaylibAudio::playMusic(Music music)
{
    const auto it = m_musics.find(music.id);
    if (it == m_musics.end()) {
        return;
    }
    stopMusic();
    it->second.looping = true;
    PlayMusicStream(it->second);
    m_currentMusic = music.id;
}

void RaylibAudio::stopMusic()
{
    const auto it = m_musics.find(m_currentMusic);
    if (it != m_musics.end()) {
        StopMusicStream(it->second);
    }
    m_currentMusic = 0;
}

void RaylibAudio::setSoundVolume(float volume)
{
    m_soundVolume = std::clamp(volume, 0.0F, 1.0F);
    for (const auto &[id, sound] : m_sounds) {
        SetSoundVolume(sound, m_soundVolume);
    }
}

void RaylibAudio::setMusicVolume(float volume)
{
    m_musicVolume = std::clamp(volume, 0.0F, 1.0F);
    for (const auto &[id, music] : m_musics) {
        SetMusicVolume(music, m_musicVolume);
    }
}

void RaylibAudio::update()
{
    const auto it = m_musics.find(m_currentMusic);
    if (it != m_musics.end()) {
        UpdateMusicStream(it->second);
    }
}

} // namespace client::audio
