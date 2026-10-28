/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** InputSystem
*/

#ifndef CLIENT_SYSTEMS_INPUTSYSTEM_HPP
#define CLIENT_SYSTEMS_INPUTSYSTEM_HPP

#include "graphics/IGraphics.hpp"

#include <cstdint>

namespace client {

// Actions held this frame, as Action bits.
[[nodiscard]] std::uint8_t inputSystem(const gfx::IGraphics &graphics);

} // namespace client

#endif // CLIENT_SYSTEMS_INPUTSYSTEM_HPP