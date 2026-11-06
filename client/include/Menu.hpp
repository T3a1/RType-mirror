/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Menu
*/

#ifndef CLIENT_MENU_HPP
#define CLIENT_MENU_HPP

#include "audio/SoundBoard.hpp"
#include "graphics/IGraphics.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <span>
#include <string>

namespace client {

/**
 * @struct MenuKeys
 * @brief The menu keys pressed this frame, not those held.
 * @param up Move up.
 * @param down Move down.
 * @param left Move left.
 * @param right Move right.
 * @param confirm Choose the selected item.
 * @param back Go back.
 **/
struct MenuKeys {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
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
    bool m_left = false;
    bool m_right = false;
    bool m_confirm = false;
    bool m_back = false;
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
 * @struct MenuCursor
 * @brief The state a menu keeps between two frames.
 * @param selected The widget selected with the keyboard.
 * @param count How many widgets the menu had at the last frame.
 **/
struct MenuCursor {
    std::size_t selected = 0;
    std::size_t count = 0;
};

/**
 * @class MenuPanel
 * @brief One frame of a menu: create it, then call one method per widget, in
 * the order of the keyboard selection.
 *
 * The panel numbers the widgets, moves the selection with Up and Down, reads
 * the keyboard and the mouse, and plays the menu sounds. A screen only says
 * what it shows and reacts to what the widgets return.
 **/
class MenuPanel {
  public:
    /**
     * @brief Starts a frame of a menu and moves its selection.
     * @param graphics The graphics backend, which must outlive the panel.
     * @param keys The menu keys of this frame.
     * @param sounds The sounds of the game, which must outlive the panel.
     * @param cursor The state of this menu, which must outlive the panel.
     **/
    MenuPanel(gfx::IGraphics &graphics, const MenuKeys &keys,
              audio::SoundBoard &sounds, MenuCursor &cursor);
    /**
     * @brief Ends the frame: keeps the number of widgets for the next one.
     **/
    ~MenuPanel();

    MenuPanel(const MenuPanel &) = delete;
    MenuPanel &operator=(const MenuPanel &) = delete;
    MenuPanel(MenuPanel &&) = delete;
    MenuPanel &operator=(MenuPanel &&) = delete;

    /**
     * @brief Draws a button. Choosing it plays MenuConfirm.
     * @param rect Where it is, in screen pixels.
     * @param label Its text.
     * @param enabled Whether it can be chosen; it can still be selected.
     * @return Whether it was chosen this frame.
     **/
    [[nodiscard]] bool button(const gfx::Rect &rect, const std::string &label,
                              bool enabled = true);
    /**
     * @brief Draws a button that goes back. Choosing it plays MenuBack.
     * @param rect Where it is, in screen pixels.
     * @param label Its text.
     * @return Whether it was chosen this frame.
     **/
    [[nodiscard]] bool backButton(const gfx::Rect &rect,
                                  const std::string &label);
    /**
     * @brief Draws a checkbox. Changing it plays MenuConfirm.
     * @param rect Where it is, in screen pixels.
     * @param label Its text.
     * @param value Whether it is checked, updated.
     * @return Whether it changed this frame.
     **/
    bool checkbox(const gfx::Rect &rect, const std::string &label, bool &value);
    /**
     * @brief Draws a slider: Left and Right move it by 5%, and a click on its
     * bar puts it there. Changing it plays MenuMove.
     * @param rect Where it is, in screen pixels.
     * @param label Its text.
     * @param value Its value from 0 to 1, updated.
     * @return Whether it changed this frame.
     **/
    bool slider(const gfx::Rect &rect, const std::string &label, float &value);
    /**
     * @brief Draws a row of tabs: Left and Right change the tab when the row
     * is selected, and a click chooses one. Changing it plays MenuMove.
     * @param rect Where the row is, in screen pixels.
     * @param labels The text of each tab.
     * @param current The index of the current tab, updated.
     **/
    void tabs(const gfx::Rect &rect, std::span<const std::string> labels,
              std::size_t &current);

  private:
    /**
     * @brief Gives its number to the next widget.
     * @return Whether the keyboard selection is on it.
     **/
    [[nodiscard]] bool nextIsSelected();
    /**
     * @brief Draws a button and plays a sound when it is chosen.
     * @param rect Where it is, in screen pixels.
     * @param label Its text.
     * @param enabled Whether it can be chosen.
     * @param sound The sound played when it is chosen.
     * @return Whether it was chosen this frame.
     **/
    [[nodiscard]] bool pressButton(const gfx::Rect &rect,
                                   const std::string &label, bool enabled,
                                   audio::SoundEvent sound);

    gfx::IGraphics *m_graphics;
    MenuKeys m_keys;
    audio::SoundBoard *m_sounds;
    MenuCursor *m_cursor;
    std::size_t m_next = 0;
};

} // namespace client

#endif // CLIENT_MENU_HPP
