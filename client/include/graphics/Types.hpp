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

/**
 * @struct Color
 * @brief An RGBA color.
 * @param r The red component, 0 to 255.
 * @param g The green component, 0 to 255.
 * @param b The blue component, 0 to 255.
 * @param a The opacity, from 0 (invisible) to 255 (opaque).
 **/
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

/** @brief The colors the client uses in several places. */
namespace colors {

/** @brief Black. */
constexpr Color Black = rgb(0, 0, 0);
/** @brief White. */
constexpr Color White = rgb(255, 255, 255);
/** @brief A soft white, for text. */
constexpr Color OffWhite = rgb(245, 245, 245);
/** @brief Dark gray. */
constexpr Color DarkGray = rgb(80, 80, 80);
/** @brief Green. */
constexpr Color Green = rgb(0, 228, 48);
/** @brief Red. */
constexpr Color Red = rgb(230, 41, 55);
/** @brief Sky blue. */
constexpr Color SkyBlue = rgb(102, 191, 255);

} // namespace colors

/**
 * @struct Rect
 * @brief An axis-aligned rectangle.
 * @param x The left edge.
 * @param y The top edge.
 * @param width Its width.
 * @param height Its height.
 **/
struct Rect {
    float x = 0.0F;
    float y = 0.0F;
    float width = 0.0F;
    float height = 0.0F;
};

/**
 * @struct Texture
 * @brief A texture loaded by an IGraphics backend.
 *
 * It is only a handle: the backend owns the pixels, and frees them in
 * IGraphics::unloadTexture() or when it is destroyed.
 * @param id What identifies it in the backend; 0 is no texture.
 * @param width Its width, in pixels.
 * @param height Its height, in pixels.
 **/
struct Texture {
    std::uint32_t id = 0;
    int width = 0;
    int height = 0;
};

/**
 * @enum TextureFilter
 * @brief How a texture is sampled when it is drawn bigger or smaller.
 * @param Smooth Bilinear: blends neighboring pixels, for photos.
 * @param Pixelated Nearest pixel: keeps pixel art sharp.
 **/
enum class TextureFilter : std::uint8_t {
    Smooth,
    Pixelated,
};

/** @brief The character IGraphics::takeTextInput() gives for the Backspace key.
 */
inline constexpr char32_t TextBackspace = U'\b';

/**
 * @enum Key
 * @brief The keyboard keys the client can read. A to Z and Num0 to Num9
 * are contiguous. Backends that cannot see a key (ncurses has no Shift,
 * Ctrl or Alt alone) never report it held.
 * @param Up The up arrow.
 * @param Down The down arrow.
 * @param Left The left arrow.
 * @param Right The right arrow.
 * @param Space The space bar.
 * @param Enter The Enter key.
 * @param Tab The Tab key.
 * @param LeftShift The left Shift key.
 * @param RightShift The right Shift key.
 * @param LeftControl The left Ctrl key.
 * @param RightControl The right Ctrl key.
 * @param LeftAlt The left Alt key.
 * @param RightAlt The right Alt key.
 * @param A The letter A.
 * @param B The letter B.
 * @param C The letter C.
 * @param D The letter D.
 * @param E The letter E.
 * @param F The letter F.
 * @param G The letter G.
 * @param H The letter H.
 * @param I The letter I.
 * @param J The letter J.
 * @param K The letter K.
 * @param L The letter L.
 * @param M The letter M.
 * @param N The letter N.
 * @param O The letter O.
 * @param P The letter P.
 * @param Q The letter Q.
 * @param R The letter R.
 * @param S The letter S.
 * @param T The letter T.
 * @param U The letter U.
 * @param V The letter V.
 * @param W The letter W.
 * @param X The letter X.
 * @param Y The letter Y.
 * @param Z The letter Z.
 * @param Num0 The digit 0.
 * @param Num1 The digit 1.
 * @param Num2 The digit 2.
 * @param Num3 The digit 3.
 * @param Num4 The digit 4.
 * @param Num5 The digit 5.
 * @param Num6 The digit 6.
 * @param Num7 The digit 7.
 * @param Num8 The digit 8.
 * @param Num9 The digit 9.
 * @param Escape The Escape key: it goes back in the menus and cannot be bound
 * to a control.
 * @param Count The number of keys, not a key.
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
    Escape,
    /** @brief The number of keys, not a key. */
    Count,
};

/** @brief The number of keys in Key. */
inline constexpr std::size_t KeyCount = static_cast<std::size_t>(Key::Count);

} // namespace client::gfx

#endif // CLIENT_GRAPHICS_TYPES_HPP
