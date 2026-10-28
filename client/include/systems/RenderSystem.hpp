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

// Draws every entity with a Transform and a Sprite, layer by layer, in
// world units: call it between beginWorld() and endWorld().
void renderSystem(engine::Registry &registry, gfx::IGraphics &graphics);

} // namespace client

#endif // CLIENT_SYSTEMS_RENDERSYSTEM_HPP
