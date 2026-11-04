/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** ShipOwner
*/

#ifndef CLIENT_COMPONENTS_SHIPOWNER_HPP
#define CLIENT_COMPONENTS_SHIPOWNER_HPP

#include <cstdint>

namespace client {

/**
 * @struct ShipOwner
 * @brief The player a ship belongs to, from the players block of the
 * snapshots (or Welcome for the local ship).
 * @param slot The player's slot, from 1 to 4.
 **/
struct ShipOwner {
    std::uint8_t slot = 0;
};

} // namespace client

#endif // CLIENT_COMPONENTS_SHIPOWNER_HPP
