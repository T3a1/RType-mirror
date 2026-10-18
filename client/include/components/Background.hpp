/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Background
*/

#ifndef CLIENT_COMPONENTS_BACKGROUND_HPP
#define CLIENT_COMPONENTS_BACKGROUND_HPP

#include <raylib.h>

namespace client {

/**
 * @brief A horizontally tileable image that fills the window and scrolls
 * forever.
 *
 * The texture is only a handle: whoever loaded it unloads it.
 */
struct Background {
    /** @brief Image to scroll, its right edge must match its left edge. */
    Texture2D texture{};
    /** @brief Scroll speed in texture pixels per second, positive scrolls to
     * the left. */
    float speed = 0.0F;
    /** @brief Texture column drawn at the left edge of the window, in [0,
     * width). */
    float offset = 0.0F;
};

} // namespace client

#endif // CLIENT_COMPONENTS_BACKGROUND_HPP
