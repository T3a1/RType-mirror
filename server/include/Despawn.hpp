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

/**
 * @brief Destroys an entity at the end of the tick and, if it has a
 * NetworkId, sends GameEvent{EntityDestroyed} to every client. Every system
 * that removes an entity goes through it.
 * @param registry The registry holding the entity.
 * @param entity The entity to destroy; a dead entity is ignored.
 * @param outbound The queue the event is pushed to.
 **/
void despawn(engine::Registry &registry, engine::Entity entity,
             Outbound &outbound);

} // namespace server

#endif // SERVER_DESPAWN_HPP
