/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Types
*/

#ifndef CLIENT_GRAPHICS_TYPES_HPP
#define CLIENT_GRAPHICS_TYPES_HPP

#include <cstdint>

namespace client::gfx {

/** @brief An RGBA color, 0 to 255 per channel. */
struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

/**
 * @brief Builds an opaque color.
 * @param red The red component, 0 to 255.
 * @param green The green component, 0 to 255.
 * @param blue The blue component, 0 to 255.
 * @return The color.
 **/
constexpr Color rgb(std::uint8_t red, std::uint8_t green, std::uint8_t blue)
{
    return {.r = red, .g = green, .b = blue, .a = 255};
}

namespace colors {

constexpr Color Black = rgb(0, 0, 0);
constexpr Color White = rgb(255, 255, 255);
constexpr Color OffWhite = rgb(245, 245, 245);
constexpr Color DarkGray = rgb(80, 80, 80);
constexpr Color Green = rgb(0, 228, 48);
constexpr Color Red = rgb(230, 41, 55);
constexpr Color SkyBlue = rgb(102, 191, 255);

} // namespace colors

/** @brief An axis-aligned rectangle, (x, y) being its top-left corner. */
struct Rect {
    float x = 0.0F;
    float y = 0.0F;
    float width = 0.0F;
    float height = 0.0F;
};

/**
 * @brief A texture loaded by an IGraphics backend.
 *
 * It is only a handle: the backend owns the pixels, and frees them in
 * IGraphics::unloadTexture() or when it is destroyed. id 0 is no texture.
 */
struct Texture {
    std::uint32_t id = 0;
    int width = 0;
    int height = 0;
};

/** @brief The keyboard keys the client reads. */
enum class Key : std::uint8_t {
    Up,
    Down,
    Left,
    Right,
    Space,
};

} // namespace client::gfx

#endif // CLIENT_GRAPHICS_TYPES_HPP
