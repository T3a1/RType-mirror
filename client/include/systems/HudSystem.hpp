/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** HudSystem
*/

#ifndef CLIENT_SYSTEMS_HUDSYSTEM_HPP
#define CLIENT_SYSTEMS_HUDSYSTEM_HPP

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
 * @brief Draws the end of the game in the middle of the window: "GAME OVER",
 * the score of every player, the team's total and a button back to the lobby.
 * It is drawn in screen pixels: call it after endWorld().
 * @param state The sync state, for the players and the local slot.
 * @param graphics The graphics backend to draw with and read the mouse from.
 * @param confirm Whether the keyboard confirmed the button.
 * @return Whether the player chose to go back to the lobby.
 **/
[[nodiscard]] bool gameOverScreen(const SyncState &state,
                                  gfx::IGraphics &graphics, bool confirm);

} // namespace client

#endif // CLIENT_SYSTEMS_HUDSYSTEM_HPP
