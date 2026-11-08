/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Playfield
*/

#ifndef COMMON_PLAYFIELD_HPP
#define COMMON_PLAYFIELD_HPP

#include "engine/Vec2.hpp"

#include <algorithm>

namespace rtype {

/**
 * @brief The width of the playfield in units.
 */
inline constexpr float PlayfieldWidth = 1920.0F;

/**
 * @brief The height of the playfield in units.
 */
inline constexpr float PlayfieldHeight = 1080.0F;

/**
 * @brief The collision size of a player ship, which also keeps it inside the
 * playfield, in units.
 */
inline constexpr engine::Vec2f PlayerShipSize{.x = 64.0F, .y = 64.0F};

/**
 * @brief Clamps the center of an entity so that the whole entity stays
 * inside the playfield.
 * @param position The center of the entity.
 * @param size The size of the entity.
 * @return The clamped center.
 */
constexpr engine::Vec2f keepInPlayfield(engine::Vec2f position,
                                        engine::Vec2f size)
{
    return {
        .x = std::clamp(position.x, size.x / 2.0F,
                        PlayfieldWidth - (size.x / 2.0F)),
        .y = std::clamp(position.y, size.y / 2.0F,
                        PlayfieldHeight - (size.y / 2.0F)),
    };
}

} // namespace rtype

#endif // COMMON_PLAYFIELD_HPP