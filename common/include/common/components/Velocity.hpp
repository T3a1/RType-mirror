/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Velocity
*/

#ifndef COMMON_COMPONENTS_VELOCITY_HPP
#define COMMON_COMPONENTS_VELOCITY_HPP

#include "engine/Vec2.hpp"

namespace rtype {

// Speed in playfield units per second.
struct Velocity {
    engine::Vec2f value;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_VELOCITY_HPP
