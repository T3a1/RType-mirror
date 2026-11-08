/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BossPartSystem
*/

#ifndef SERVER_SYSTEMS_BOSSPARTSYSTEM_HPP
#define SERVER_SYSTEMS_BOSSPARTSYSTEM_HPP

#include "Despawn.hpp"
#include "engine/Clock.hpp"
#include "engine/Registry.hpp"

namespace server {

/**
 * @brief Keeps every BossPart on its boss, at its offset, waving up and
 * down by its swing, and destroys the parts whose boss is gone. Runs after
 * movementSystem() and before collisionSystem().
 * @param registry The registry holding the bosses and their parts.
 * @param outbound Where the destruction of a part is announced.
 * @param dt The time elapsed since the last tick.
 **/
void bossPartSystem(engine::Registry &registry, Outbound &outbound,
                    engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_BOSSPARTSYSTEM_HPP
