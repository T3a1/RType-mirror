/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** PowerUp
*/

#ifndef SERVER_COMPONENTS_POWERUP_HPP
#define SERVER_COMPONENTS_POWERUP_HPP

#include "common/components/PowerUpKind.hpp"

namespace server {

/**
 * @struct PowerUp
 * @brief A bonus floating to the left: the player who touches it gets its
 * boost for a while.
 * @param kind The boost it gives.
 **/
struct PowerUp {
    rtype::PowerUpKind kind = rtype::PowerUpKind::Spread;
};

} // namespace server

#endif // SERVER_COMPONENTS_POWERUP_HPP
