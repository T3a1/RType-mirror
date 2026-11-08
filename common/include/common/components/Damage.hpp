/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Damage
*/

#ifndef COMMON_COMPONENTS_DAMAGE_HPP
#define COMMON_COMPONENTS_DAMAGE_HPP

namespace rtype {

/**
 * @struct Damage
 * @brief What an entity deals to what it collides with.
 * @param points The health points removed.
 **/
struct Damage {
    int points = 1;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_DAMAGE_HPP
