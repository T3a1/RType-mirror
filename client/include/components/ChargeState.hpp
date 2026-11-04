/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** ChargeState
*/

#ifndef CLIENT_COMPONENTS_CHARGESTATE_HPP
#define CLIENT_COMPONENTS_CHARGESTATE_HPP

namespace client {

/**
 * @struct ChargeState
 * @brief Whether a player ship is charging its shot, from the Charging
 * flag of the last snapshot.
 * @param charging Whether the fire key is held long enough to show it.
 **/
struct ChargeState {
    bool charging = false;
};

} // namespace client

#endif // CLIENT_COMPONENTS_CHARGESTATE_HPP
