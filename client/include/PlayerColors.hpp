/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** PlayerColors
*/

#ifndef CLIENT_PLAYERCOLORS_HPP
#define CLIENT_PLAYERCOLORS_HPP

#include "graphics/Types.hpp"

#include <cstdint>

namespace client {

/**
 * @brief The color of a player, for its ship label and its HUD line.
 *
 * Taken from the Okabe-Ito palette, which color-blind players can tell
 * apart, and close to the ship's sprite: sky blue (cyan ship), reddish
 * purple (magenta ship), bluish green (green ship), vermillion (red ship).
 * Never the only cue: labels and the HUD also show the player number.
 * @param slot The player's slot, from 1 to 4.
 * @return Its color, off-white for an unknown slot.
 **/
constexpr gfx::Color playerColor(std::uint8_t slot)
{
    switch (slot) {
    case 1:
        return gfx::rgb(86, 180, 233);
    case 2:
        return gfx::rgb(204, 121, 167);
    case 3:
        return gfx::rgb(0, 158, 115);
    case 4:
        return gfx::rgb(213, 94, 0);
    default:
        return gfx::colors::OffWhite;
    }
}

} // namespace client

#endif // CLIENT_PLAYERCOLORS_HPP
