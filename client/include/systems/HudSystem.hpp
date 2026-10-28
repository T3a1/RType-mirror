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
 * @brief Draws a health bar above every player ship, from its Health and
 * Transform. It is drawn in playfield units: call it between beginWorld()
 * and endWorld().
 * @param registry The client's registry.
 * @param graphics The graphics backend to draw with.
 **/
void healthBarSystem(engine::Registry &registry, gfx::IGraphics &graphics);

/**
 * @brief Draws the score of every player in the top-left corner, the local
 * player highlighted and marked "(you)". It is drawn in screen pixels: call
 * it after endWorld().
 * @param state The sync state, for the players and the local slot.
 * @param graphics The graphics backend to draw with.
 **/
void hudSystem(const SyncState &state, gfx::IGraphics &graphics);

} // namespace client

#endif // CLIENT_SYSTEMS_HUDSYSTEM_HPP
