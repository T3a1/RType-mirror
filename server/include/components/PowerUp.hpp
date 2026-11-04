/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** PowerUp
*/

#ifndef SERVER_COMPONENTS_POWERUP_HPP
#define SERVER_COMPONENTS_POWERUP_HPP

#include "components/ShotStyles.hpp"

namespace server {

/**
 * @struct PowerUp
 * @brief A bonus floating to the left: the player who touches it gets its
 * shot style, or one more level of it.
 * @param kind The shot style it gives.
 **/
struct PowerUp {
    PowerUpKind kind = PowerUpKind::Spread;
};

} // namespace server

#endif // SERVER_COMPONENTS_POWERUP_HPP
