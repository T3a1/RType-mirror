/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NullAudio
*/

#include "NullAudio.hpp"

namespace client::audio {

std::unique_ptr<IAudio> createAudio() { return std::make_unique<NullAudio>(); }

} // namespace client::audio
