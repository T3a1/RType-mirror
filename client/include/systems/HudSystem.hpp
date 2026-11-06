/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** HudSystem
*/

#ifndef CLIENT_SYSTEMS_HUDSYSTEM_HPP
#define CLIENT_SYSTEMS_HUDSYSTEM_HPP

#include "Menu.hpp"
#include "engine/Registry.hpp"
#include "graphics/IGraphics.hpp"
#include "systems/NetworkSyncSystem.hpp"

namespace client {

/**
 * @brief Draws the label of every ship whose player is known ("P2", or
 * "P1 (you)" for the local ship) above it, in the player's color. It is drawn
 *in playfield units: call it between beginWorld() and endWorld().
 * @param registry The client's registry.
 * @param state The sync state, for the local slot.
 * @param graphics The graphics backend to draw with.
 **/
void playerLabelSystem(engine::Registry &registry, const SyncState &state,
                       gfx::IGraphics &graphics);

/**
 * @brief Draws the score of every player in the top-left corner, each in
 * the player's color, the local player marked "(you)", and "Auto-fire ON"
 * below them while auto-fire is on. It is drawn in screen pixels: call it
 * after endWorld().
 * @param state The sync state, for the players and the local slot.
 * @param autoFire Whether auto-fire is on.
 * @param graphics The graphics backend to draw with.
 **/
void hudSystem(const SyncState &state, bool autoFire, gfx::IGraphics &graphics);

/**
 * @brief Draws the health bar of the boss at the bottom of the window,
 * with its name, while a boss is on screen. It is drawn in screen pixels:
 * call it after endWorld().
 * @param registry The client's registry, for the boss and its Health.
 * @param graphics The graphics backend to draw with.
 **/
void bossHealthBar(engine::Registry &registry, gfx::IGraphics &graphics);

/**
 * @brief Draws the current level at the top of the window ("Level 2" or
 * "Endless mode"), and while a new level is announced, a large banner in
 * the middle ("LEVEL 2", after "Level cleared" from the second level on),
 * fading out at the end. It is drawn in screen pixels: call it after
 * endWorld().
 * @param state The sync state, for the level and its banner.
 * @param graphics The graphics backend to draw with.
 **/
void levelBannerScreen(const SyncState &state, gfx::IGraphics &graphics);

/**
 * @brief Draws the notices of the players who just joined or left, in the
 * top right corner of the window, in the player's color and fading out at the
 * end. The text names the player, so it does not rely on the color. It is
 * drawn in screen pixels: call it after endWorld().
 * @param state The sync state, for the notices.
 * @param graphics The graphics backend to draw with.
 **/
void playerNoticeSystem(const SyncState &state, gfx::IGraphics &graphics);

/**
 * @brief Draws the end of the game in the middle of the window: "GAME OVER",
 * the score of every player, the team's total and a button back to the lobby.
 * It is drawn in screen pixels: call it after endWorld().
 * @param state The sync state, for the players and the local slot.
 * @param context What the menu needs for this frame.
 * @return Whether the player chose to go back to the lobby.
 **/
[[nodiscard]] bool gameOverScreen(const SyncState &state,
                                  const MenuContext &context);

} // namespace client

#endif // CLIENT_SYSTEMS_HUDSYSTEM_HPP
