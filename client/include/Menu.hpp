/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Menu
*/

#ifndef CLIENT_MENU_HPP
#define CLIENT_MENU_HPP

#include "graphics/IGraphics.hpp"

#include <string>

namespace client {

/**
 * @struct MenuKeys
 * @brief The menu keys pressed this frame, not those held.
 * @param up Move up.
 * @param down Move down.
 * @param confirm Choose the selected item.
 * @param back Go back.
 **/
struct MenuKeys {
    bool up = false;
    bool down = false;
    bool confirm = false;
    bool back = false;
};

/**
 * @class MenuInput
 * @brief Turns the held keys of IGraphics into key presses for the menus.
 * Update it every frame, also out of the menus, so that a key held since the
 * game does not count as a press.
 **/
class MenuInput {
  public:
    /**
     * @brief Reads the keyboard.
     * @param graphics The graphics backend.
     * @return The keys pressed since the last call.
     **/
    [[nodiscard]] MenuKeys update(const gfx::IGraphics &graphics);

  private:
    bool m_up = false;
    bool m_down = false;
    bool m_confirm = false;
    bool m_back = false;
};

/**
 * @struct ButtonStyle
 * @brief How a menu button looks.
 * @param selected Whether the keyboard selection is on it.
 * @param enabled Whether it can be chosen; a disabled button is dimmed.
 **/
struct ButtonStyle {
    bool selected = false;
    bool enabled = true;
};

/**
 * @brief Draws one line of text centered around a point.
 * @param graphics The graphics backend to draw with.
 * @param text The text.
 * @param centerX The horizontal center of the text.
 * @param y The top of the text.
 * @param fontSize The font size.
 * @param color The color of the text.
 **/
void drawCentered(gfx::IGraphics &graphics, const std::string &text,
                  float centerX, float y, int fontSize, gfx::Color color);

/**
 * @brief Draws a button with a centered label, in screen pixels.
 * @param graphics The graphics backend to draw with and read the mouse from.
 * @param rect Where the button is.
 * @param label Its text.
 * @param style How it looks.
 * @return Whether it was clicked, never when disabled.
 **/
[[nodiscard]] bool menuButton(gfx::IGraphics &graphics, const gfx::Rect &rect,
                              const std::string &label, ButtonStyle style);

} // namespace client

#endif // CLIENT_MENU_HPP
