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

void healthBarSystem(engine::Registry &registry);

void hudSystem(const SyncState &state);

} // namespace client

#endif // CLIENT_SYSTEMS_HUDSYSTEM_HPP
