/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** RenderSystem
*/

#ifndef CLIENT_SYSTEMS_RENDERSYSTEM_HPP
#define CLIENT_SYSTEMS_RENDERSYSTEM_HPP

#include "engine/Registry.hpp"

namespace client {

// Draws every entity with a Transform and a Sprite, layer by layer.
// Must be called between BeginDrawing() and EndDrawing().
void renderSystem(engine::Registry &registry);

} // namespace client

#endif // CLIENT_SYSTEMS_RENDERSYSTEM_HPP
