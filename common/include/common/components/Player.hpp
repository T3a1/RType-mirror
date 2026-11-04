/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Player
*/

#ifndef COMMON_COMPONENTS_PLAYER_HPP
#define COMMON_COMPONENTS_PLAYER_HPP

#include <cstdint>

namespace rtype {

// A player dies from a single enemy shot or collision.
inline constexpr int PlayerMaxHealth = 1;

struct Player {
    std::uint8_t slot = 0;
    std::uint32_t score = 0;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_PLAYER_HPP
