/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Transform
*/

#ifndef COMMON_COMPONENTS_TRANSFORM_HPP
#define COMMON_COMPONENTS_TRANSFORM_HPP

#include "engine/Vec2.hpp"

namespace rtype {

/**
 * @struct Transform
 * @brief Where an entity is.
 * @param position The position of its center, in playfield units.
 * @param rotation Its rotation, in radians.
 **/
struct Transform {
    engine::Vec2f position;
    float rotation = 0.0;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_TRANSFORM_HPP
