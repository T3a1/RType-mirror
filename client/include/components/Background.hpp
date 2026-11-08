/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Background
*/

#ifndef CLIENT_COMPONENTS_BACKGROUND_HPP
#define CLIENT_COMPONENTS_BACKGROUND_HPP

#include "graphics/Types.hpp"

namespace client {

/**
 * @struct Background
 * @brief A horizontally tileable image that fills the window and scrolls
 * forever. Several backgrounds are drawn on top of each other, by depth:
 * a nearer layer scrolling faster than a farther one gives a feeling of
 * depth (parallax).
 *
 * The texture is only a handle: whoever loaded it unloads it.
 */
struct Background {
    /** @brief Image to scroll, its right edge must match its left edge. */
    gfx::Texture texture;
    /** @brief Scroll speed in texture pixels per second, positive scrolls to
     * the left. */
    float speed = 0.0F;
    /** @brief Texture column drawn at the left edge of the window, in [0,
     * width). */
    float offset = 0.0F;
    /** @brief Drawing order: lower depths are drawn first, under the higher
     * ones. */
    int depth = 0;
};

} // namespace client

#endif // CLIENT_COMPONENTS_BACKGROUND_HPP
