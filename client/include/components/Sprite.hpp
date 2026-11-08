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

/**
 * @struct Sprite
 * @brief How an entity is drawn, centered on its Transform.
 * @param size The drawn size, in playfield units.
 * @param tint The color the texture is multiplied by, or the color of the
 * rectangle drawn when there is no texture.
 * @param layer Higher layers are drawn on top of lower ones.
 * @param texture The sprite sheet, id 0 to draw a plain rectangle instead
 * (backends without textures, such as ncurses). Only a handle: the
 * SpriteSheets that loaded it unloads it.
 * @param source The part of the texture drawn, in texture pixels.
 **/
struct Sprite {
    engine::Vec2f size;
    gfx::Color tint = gfx::colors::White;
    int layer = 0;
    gfx::Texture texture;
    gfx::Rect source;
};

} // namespace client

#endif // CLIENT_COMPONENTS_SPRITE_HPP
