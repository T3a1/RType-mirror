/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** RenderSystem
*/

#ifndef CLIENT_SYSTEMS_RENDERSYSTEM_HPP
#define CLIENT_SYSTEMS_RENDERSYSTEM_HPP

#include "engine/Registry.hpp"
#include "graphics/IGraphics.hpp"

namespace client {

/**
 * @brief Draws every entity with a Transform and a Sprite, layer by layer, in
 * world units: call it between beginWorld() and endWorld(). A Sprite without a
 * texture is drawn as a rectangle of its tint.
 * @param registry The client's registry.
 * @param graphics The graphics backend to draw with.
 **/
void renderSystem(engine::Registry &registry, gfx::IGraphics &graphics);

} // namespace client

#endif // CLIENT_SYSTEMS_RENDERSYSTEM_HPP
