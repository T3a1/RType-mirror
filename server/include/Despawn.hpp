/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Despawn
*/

#ifndef SERVER_DESPAWN_HPP
#define SERVER_DESPAWN_HPP

#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"

namespace server {

// Destroys an entity and, if the clients know it, tells them.
void despawn(engine::Registry &registry, engine::Entity entity,
             Outbound &outbound);

} // namespace server

#endif // SERVER_DESPAWN_HPP
