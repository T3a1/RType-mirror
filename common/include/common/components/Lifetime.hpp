/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Lifetime
*/

#ifndef COMMON_COMPONENTS_LIFETIME_HPP
#define COMMON_COMPONENTS_LIFETIME_HPP

#include "engine/Clock.hpp"

namespace rtype {

/**
 * @brief Component that represents the remaining lifetime of an entity.
 * @param remaining The remaining time before the entity is destroyed.
 */
struct Lifetime {
    engine::Duration remaining{3.0F};
};

} // namespace rtype

#endif // COMMON_COMPONENTS_LIFETIME_HPP
