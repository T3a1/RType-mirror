/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Health
*/

#ifndef COMMON_COMPONENTS_HEALTH_HPP
#define COMMON_COMPONENTS_HEALTH_HPP

namespace rtype {
/**
 * @struct Health
 * @brief The health points of an entity, which dies at 0.
 * @param current The points left.
 * @param max The points it starts with.
 **/
struct Health {
    int current = 100;
    int max = 100;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_HEALTH_HPP
