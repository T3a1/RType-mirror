/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** AnimationSystem
*/

#ifndef CLIENT_SYSTEMS_ANIMATIONSYSTEM_HPP
#define CLIENT_SYSTEMS_ANIMATIONSYSTEM_HPP

#include "engine/Clock.hpp"
#include "engine/Registry.hpp"

namespace client {

/**
 * @brief Advances every Animation and shows its current frame in the
 * entity's Sprite. An entity whose clip does not loop is destroyed after
 * its last frame.
 * @param registry The client's registry.
 * @param dt The time elapsed since the last frame.
 **/
void animationSystem(engine::Registry &registry, engine::Duration dt);

/**
 * @brief Tilts the player ships that move up or down, and brings them
 * back level when they stop. Runs after the positions of the frame are
 * known (after smoothingSystem()).
 * @param registry The client's registry.
 * @param dt The time elapsed since the last frame.
 **/
void shipTiltSystem(engine::Registry &registry, engine::Duration dt);

} // namespace client

#endif // CLIENT_SYSTEMS_ANIMATIONSYSTEM_HPP
