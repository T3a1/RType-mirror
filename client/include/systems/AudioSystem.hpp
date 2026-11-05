/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** AudioSystem
*/

#ifndef CLIENT_SYSTEMS_AUDIOSYSTEM_HPP
#define CLIENT_SYSTEMS_AUDIOSYSTEM_HPP

#include "audio/SoundBoard.hpp"

namespace client {

/**
 * @brief Picks the music and keeps it playing. Runs at the end of the frame.
 * @param inGame Whether a game is running.
 * @param sounds The sounds and musics of the game.
 **/
void audioSystem(bool inGame, audio::SoundBoard &sounds);

} // namespace client

#endif // CLIENT_SYSTEMS_AUDIOSYSTEM_HPP
