/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** AudioSystem
*/

#include "systems/AudioSystem.hpp"
#include "common/components/EntityType.hpp"

namespace client {

namespace {

/**
 * @brief Returns the music to play: the boss music while a boss is alive during
 * a game.
 * @param registry The client's registry.
 * @param inGame Whether a game is running.
 * @return The music.
 **/
audio::MusicTrack pickMusic(engine::Registry &registry, bool inGame)
{
    if (!inGame) {
        return audio::MusicTrack::Menu;
    }
    bool bossAlive = false;
    registry.each<rtype::EntityType>(
        [&bossAlive](engine::Entity, const rtype::EntityType &type) {
            bossAlive = bossAlive || type == rtype::EntityType::Boss;
        });
    return bossAlive ? audio::MusicTrack::Boss : audio::MusicTrack::Game;
}

} // namespace

void audioSystem(engine::Registry &registry, bool inGame,
                 audio::SoundBoard &sounds)
{
    sounds.setMusic(pickMusic(registry, inGame));
    sounds.update();
}

} // namespace client
