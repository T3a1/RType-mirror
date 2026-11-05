/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** AudioSystem
*/

#include "systems/AudioSystem.hpp"

namespace client {

void audioSystem(bool inGame, audio::SoundBoard &sounds)
{
    sounds.setMusic(inGame ? audio::MusicTrack::Game : audio::MusicTrack::Menu);
    sounds.update();
}

} // namespace client
