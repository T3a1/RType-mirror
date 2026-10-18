/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** BackgroundRenderSystem
*/

#ifndef CLIENT_SYSTEMS_BACKGROUNDRENDERSYSTEM_HPP
#define CLIENT_SYSTEMS_BACKGROUNDRENDERSYSTEM_HPP

#include "engine/Registry.hpp"

namespace client {

/**
 * @brief Draws every Background over the whole window.
 *
 * Each Background is scaled to the window height and repeated horizontally
 * from its scrolled position until the window is covered.
 *
 * @param registry Registry holding the Background components.
 */
void backgroundRenderSystem(engine::Registry &registry);

} // namespace client

#endif // CLIENT_SYSTEMS_BACKGROUNDRENDERSYSTEM_HPP
