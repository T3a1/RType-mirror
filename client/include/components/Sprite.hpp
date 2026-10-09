/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Sprite
*/

#ifndef CLIENT_COMPONENTS_SPRITE_HPP
#define CLIENT_COMPONENTS_SPRITE_HPP

#include "engine/Vec2.hpp"

#include <raylib.h>

namespace client {

struct Sprite {
    engine::Vec2f size;
    Color tint{.r = 255, .g = 255, .b = 255, .a = 255};
    int layer = 0;
};

} // namespace client

#endif // CLIENT_COMPONENTS_SPRITE_HPP
