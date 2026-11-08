/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** MenuPages
*/

#ifndef CLIENT_MENUPAGES_HPP
#define CLIENT_MENUPAGES_HPP

#include "Controls.hpp"
#include "KeyCapture.hpp"
#include "Menu.hpp"
#include "Settings.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace client {

/**
 * @enum MenuPage
 * @brief The page shown while the player is not in a room.
 * @param Main The main menu.
 * @param Lobby The menu of the rooms.
 * @param Settings The settings of the player.
 * @param Accessibility The accessibility options of the player.
 * @param HowToPlay The rules and the keys.
 * @param Credits The authors of the game.
 **/
enum class MenuPage : std::uint8_t {
    Main,
    Lobby,
    Settings,
    Accessibility,
    HowToPlay,
    Credits
};

/**
 * @struct SettingsPageState
 * @brief What the Settings page keeps between two frames.
 * @param cursor The keyboard selection of the page.
 * @param tab The current tab: 0 Audio, 1 Keyboard, 2 Gamepad, 3 Video.
 * @param capture The key being changed, if any.
 * @param notice The text under the keys, to report a key used twice.
 **/
struct SettingsPageState {
    MenuCursor cursor;
    std::size_t tab = 0;
    KeyCapture capture;
    std::string notice;
};

/**
 * @struct MenuState
 * @brief Where the player is in the menus.
 * @param page The page shown.
 * @param mainCursor The keyboard selection of the main menu.
 * @param settings The state of the Settings page.
 * @param accessibilityCursor The keyboard selection of the Accessibility page.
 * @param howToPlayCursor The keyboard selection of the How to play page.
 * @param creditsCursor The keyboard selection of the Credits page.
 **/
struct MenuState {
    MenuPage page = MenuPage::Main;
    MenuCursor mainCursor;
    SettingsPageState settings;
    MenuCursor accessibilityCursor;
    MenuCursor howToPlayCursor;
    MenuCursor creditsCursor;
};

/**
 * @enum MainChoice
 * @brief What the player chose in the main menu.
 * @param None Nothing this frame.
 * @param Play Go to the lobby.
 * @param Settings Show the settings of the player.
 * @param Accessibility Show the accessibility options of the player.
 * @param HowToPlay Show the rules and the keys.
 * @param Credits Show the authors of the game.
 * @param Quit Close the game.
 **/
enum class MainChoice : std::uint8_t {
    None,
    Play,
    Settings,
    Accessibility,
    HowToPlay,
    Credits,
    Quit
};

/**
 * @brief Draws the main menu, in screen pixels.
 * @param context What the menu needs for this frame.
 * @param cursor The keyboard selection of this menu, updated.
 * @param connected Whether the client is connected: Play is disabled until
 * then.
 * @return What the player chose.
 **/
[[nodiscard]] MainChoice mainMenuPage(const MenuContext &context,
                                      MenuCursor &cursor, bool connected);

/**
 * @struct SettingsResult
 * @brief What the player did on a page of settings this frame.
 * @param changed Whether a setting changed.
 * @param controlsChanged Whether the keys changed.
 * @param videoChanged Whether the window or the frame rate changed.
 * @param back Whether the player chose to go back.
 **/
struct SettingsResult {
    bool changed = false;
    bool controlsChanged = false;
    bool videoChanged = false;
    bool back = false;
};

/**
 * @brief Draws the settings of the player, in screen pixels, in four tabs.
 * On Audio, the sound volume plays a sound of the game when it changes, so the
 * player can hear it. On Keyboard and Gamepad, each control has two keys or
 * buttons to change. Video holds the window and the frame rate.
 * @param context What the menu needs for this frame.
 * @param state The state of this page, updated.
 * @param settings The settings, updated.
 * @param controls The keys bound to each control on each device, updated.
 * @return What the player did.
 **/
[[nodiscard]] SettingsResult settingsPage(const MenuContext &context,
                                          SettingsPageState &state,
                                          Settings &settings,
                                          PlayerControls &controls);

/**
 * @brief Draws one checkbox per line of Toggles, in screen pixels.
 * @param context What the menu needs for this frame.
 * @param cursor The keyboard selection of this menu, updated.
 * @param settings The settings, updated.
 * @return What the player did.
 **/
[[nodiscard]] SettingsResult accessibilityPage(const MenuContext &context,
                                               MenuCursor &cursor,
                                               Settings &settings);

/**
 * @brief Returns the lines of the How to play page. The keys come from the
 * controls, so they stay true with a custom controls file.
 * @param controls The keys bound to each control.
 * @return The lines, in order.
 **/
[[nodiscard]] std::array<std::string, 6>
howToPlayLines(const Controls &controls);

/**
 * @brief Draws the rules and the keys, in screen pixels.
 * @param context What the menu needs for this frame.
 * @param cursor The keyboard selection of this menu, updated.
 * @param controls The keys bound to each control.
 * @return Whether the player chose to go back.
 **/
[[nodiscard]] bool howToPlayPage(const MenuContext &context, MenuCursor &cursor,
                                 const Controls &controls);

/**
 * @brief Draws the authors of the game and what it is made with, in screen
 * pixels.
 * @param context What the menu needs for this frame.
 * @param cursor The keyboard selection of this menu, updated.
 * @return Whether the player chose to go back.
 **/
[[nodiscard]] bool creditsPage(const MenuContext &context, MenuCursor &cursor);

} // namespace client

#endif // CLIENT_MENUPAGES_HPP
