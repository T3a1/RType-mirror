/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Projectile
*/

#ifndef COMMON_COMPONENTS_PROJECTILE_HPP
#define COMMON_COMPONENTS_PROJECTILE_HPP

#include "engine/Entity.hpp"

namespace rtype {

/**
 * @brief Component that represents a projectile entity.
 * @param owner The entity that owns the projectile.
 */
struct Projectile {
    engine::Entity owner{};
};

} // namespace rtype

#endif // COMMON_COMPONENTS_PROJECTILE_HPP
