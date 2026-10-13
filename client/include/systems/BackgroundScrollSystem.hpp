/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** BackgroundScrollSystem
*/

#ifndef CLIENT_SYSTEMS_BACKGROUNDSCROLLSYSTEM_HPP
#define CLIENT_SYSTEMS_BACKGROUNDSCROLLSYSTEM_HPP

#include "engine/Clock.hpp"
#include "engine/Registry.hpp"

namespace client {

/**
 * @brief Scrolls every Background.
 *
 * Advances the offset of each Background by speed * dt, wrapping around the
 * texture width so the offset always stays in [0, width).
 *
 * @param registry Registry holding the Background components.
 * @param dt Time elapsed since the previous frame.
 */
void backgroundScrollSystem(engine::Registry &registry, engine::Duration dt);

} // namespace client

#endif // CLIENT_SYSTEMS_BACKGROUNDSCROLLSYSTEM_HPP
