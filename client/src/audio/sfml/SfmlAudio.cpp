/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SfmlAudio
*/

#include "SfmlAudio.hpp"

namespace client::audio {

std::unique_ptr<IAudio> createAudio()
{
    if (!sf::PlaybackDevice::getDefaultDevice()) {
        BB_WARN("No audio device: the game runs without sound");
        return std::make_unique<NullAudio>();
    }
    return std::make_unique<SfmlAudio>();
}

Sound SfmlAudio::loadSound(const std::string &path)
{
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(m_directory / path)) {
        BB_WARN("Cannot load the sound " << path);
        return {};
    }
    auto loaded = std::make_unique<LoadedSound>(buffer);
    loaded->sound.setVolume(m_soundVolume);
    const std::uint32_t id = m_nextId++;
    m_sounds.emplace(id, std::move(loaded));
    return {.id = id};
}

Music SfmlAudio::loadMusic(const std::string &path)
{
    auto music = std::make_unique<sf::Music>();
    if (!music->openFromFile(m_directory / path)) {
        BB_WARN("Cannot load the music " << path);
        return {};
    }
    music->setLooping(true);
    music->setVolume(m_musicVolume);
    const std::uint32_t id = m_nextId++;
    m_musics.emplace(id, std::move(music));
    return {.id = id};
}

void SfmlAudio::playSound(Sound sound)
{
    const auto it = m_sounds.find(sound.id);
    if (it != m_sounds.end()) {
        it->second->sound.play();
    }
}

void SfmlAudio::playMusic(Music music)
{
    const auto it = m_musics.find(music.id);
    if (it == m_musics.end()) {
        return;
    }
    stopMusic();
    it->second->play();
    m_currentMusic = music.id;
}

void SfmlAudio::stopMusic()
{
    const auto it = m_musics.find(m_currentMusic);
    if (it != m_musics.end()) {
        it->second->stop();
    }
    m_currentMusic = 0;
}

void SfmlAudio::setSoundVolume(float volume)
{
    m_soundVolume = std::clamp(volume, 0.0F, 1.0F) * 100.0F;
    for (const auto &[id, loaded] : m_sounds) {
        loaded->sound.setVolume(m_soundVolume);
    }
}

void SfmlAudio::setMusicVolume(float volume)
{
    m_musicVolume = std::clamp(volume, 0.0F, 1.0F) * 100.0F;
    for (const auto &[id, music] : m_musics) {
        music->setVolume(m_musicVolume);
    }
}

// SFML streams the music on its own thread.
void SfmlAudio::update() {}

} // namespace client::audio
