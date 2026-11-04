/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Types
*/

#ifndef CLIENT_GRAPHICS_TYPES_HPP
#define CLIENT_GRAPHICS_TYPES_HPP

#include <cstddef>
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

/** @brief How a texture is sampled when it is drawn bigger or smaller. */
enum class TextureFilter : std::uint8_t {
    /** @brief Bilinear: blends neighboring pixels, for photos. */
    Smooth,
    /** @brief Nearest pixel: keeps pixel art sharp. */
    Pixelated,
};

/**
 * @brief The keyboard keys the client can read. A to Z and Num0 to Num9
 * are contiguous. Backends that cannot see a key (ncurses has no Shift,
 * Ctrl or Alt alone) never report it held.
 */
enum class Key : std::uint8_t {
    Up,
    Down,
    Left,
    Right,
    Space,
    Enter,
    Tab,
    LeftShift,
    RightShift,
    LeftControl,
    RightControl,
    LeftAlt,
    RightAlt,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    /** @brief The number of keys, not a key. */
    Count,
};

/** @brief The number of keys in Key. */
inline constexpr std::size_t KeyCount = static_cast<std::size_t>(Key::Count);

} // namespace client::gfx

#endif // CLIENT_GRAPHICS_TYPES_HPP
