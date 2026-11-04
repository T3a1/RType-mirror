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
 * the player's color, the local player marked "(you)". It is drawn in
 * screen pixels: call it after endWorld().
 * @param state The sync state, for the players and the local slot.
 * @param graphics The graphics backend to draw with.
 **/
void hudSystem(const SyncState &state, gfx::IGraphics &graphics);

} // namespace client

#endif // CLIENT_SYSTEMS_HUDSYSTEM_HPP
