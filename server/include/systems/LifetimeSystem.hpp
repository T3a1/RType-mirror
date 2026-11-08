/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** LifetimeSystem
*/

#ifndef SERVER_SYSTEMS_LIFETIMESYSTEM_HPP
#define SERVER_SYSTEMS_LIFETIMESYSTEM_HPP

#include "Despawn.hpp"
#include "common/components/Lifetime.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"

namespace server {

/**
 * @brief Destroys every entity whose lifetime has run out.
 * @param registry The registry holding the entities.
 * @param dt The time elapsed since the last tick.
 * @param outbound Where the destruction is announced to the clients.
 **/
void lifetimeSystem(engine::Registry &registry, engine::Duration dt,
                    Outbound &outbound);

} // namespace server

#endif // SERVER_SYSTEMS_LIFETIMESYSTEM_HPP
