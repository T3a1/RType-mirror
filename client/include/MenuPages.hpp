/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** MenuPages
*/

#ifndef CLIENT_MENUPAGES_HPP
#define CLIENT_MENUPAGES_HPP

#include "Controls.hpp"
#include "Menu.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace client {

/**
 * @enum MenuPage
 * @brief The page shown while the player is not in a room.
 * @param Main The main menu.
 * @param Lobby The menu of the rooms.
 * @param HowToPlay The rules and the keys.
 * @param Credits The authors of the game.
 **/
enum class MenuPage : std::uint8_t { Main, Lobby, HowToPlay, Credits };

/**
 * @struct MenuState
 * @brief Where the player is in the menus.
 * @param page The page shown.
 * @param mainCursor The keyboard selection of the main menu.
 * @param howToPlayCursor The keyboard selection of the How to play page.
 * @param creditsCursor The keyboard selection of the Credits page.
 **/
struct MenuState {
    MenuPage page = MenuPage::Main;
    MenuCursor mainCursor;
    MenuCursor howToPlayCursor;
    MenuCursor creditsCursor;
};

/**
 * @enum MainChoice
 * @brief What the player chose in the main menu.
 * @param None Nothing this frame.
 * @param Play Go to the lobby.
 * @param HowToPlay Show the rules and the keys.
 * @param Credits Show the authors of the game.
 * @param Quit Close the game.
 **/
enum class MainChoice : std::uint8_t { None, Play, HowToPlay, Credits, Quit };

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
 * @brief Returns the lines of the How to play page. The keys come from the
 * controls, so they stay true with a custom controls file.
 * @param controls The keys bound to each control.
 * @return The lines, in order.
 **/
[[nodiscard]] std::array<std::string, 5>
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
