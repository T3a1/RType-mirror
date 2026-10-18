/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** HudSystem
*/

#ifndef CLIENT_SYSTEMS_HUDSYSTEM_HPP
#define CLIENT_SYSTEMS_HUDSYSTEM_HPP

#include "engine/Registry.hpp"
#include "systems/NetworkSyncSystem.hpp"

namespace client {

/**
 * @brief Draws a health bar above every player ship, from its Health and
 * Transform. It is drawn in playfield units: call it between BeginMode2D()
 * and EndMode2D().
 * @param registry The client's registry.
 **/
void healthBarSystem(engine::Registry &registry);

/**
 * @brief Draws the score of every player in the top-left corner, the local
 * player highlighted and marked "(you)". It is drawn in screen pixels: call
 * it after EndMode2D().
 * @param state The sync state, for the players and the local slot.
 **/
void hudSystem(const SyncState &state);

} // namespace client

#endif // CLIENT_SYSTEMS_HUDSYSTEM_HPP
