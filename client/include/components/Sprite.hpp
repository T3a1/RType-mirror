/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Sprite
*/

#ifndef CLIENT_COMPONENTS_SPRITE_HPP
#define CLIENT_COMPONENTS_SPRITE_HPP

#include "engine/Vec2.hpp"
#include "graphics/Types.hpp"

namespace client {

struct Sprite {
    engine::Vec2f size;
    gfx::Color tint = gfx::colors::White;
    int layer = 0;
};

} // namespace client

#endif // CLIENT_COMPONENTS_SPRITE_HPP
