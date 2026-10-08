/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Projectile
*/

#ifndef COMMON_COMPONENTS_PROJECTILE_HPP
#define COMMON_COMPONENTS_PROJECTILE_HPP

#include "engine/Clock.hpp"
#include "engine/Entity.hpp"

namespace rtype {

struct Projectile {
    engine::Entity owner{};
    engine::Duration lifetime{3.0F};
};

} // namespace rtype

#endif // COMMON_COMPONENTS_PROJECTILE_HPP
