/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BoundarySystem
*/

#ifndef SERVER_SYSTEMS_BOUNDARYSYSTEM_HPP
#define SERVER_SYSTEMS_BOUNDARYSYSTEM_HPP

#include "Despawn.hpp"
#include "common/Playfield.hpp"
#include "common/components/Collider.hpp"
#include "common/components/Transform.hpp"
#include "components/DestroyOffScreen.hpp"
#include "components/KeepInPlayfield.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"

#include <algorithm>

namespace server {

/**
 * @brief Applies the playfield edges: destroys the DestroyOffScreen entities
 *that left it, and clamps the KeepInPlayfield ones inside it.
 * @param registry The registry to update.
 * @param outbound Where the destructions are announced to the clients.
 **/
void boundarySystem(engine::Registry &registry, Outbound &outbound);

} // namespace server

#endif // SERVER_SYSTEMS_BOUNDARYSYSTEM_HPP
