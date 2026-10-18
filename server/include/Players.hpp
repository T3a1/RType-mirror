/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Players
*/

#ifndef SERVER_PLAYERS_HPP
#define SERVER_PLAYERS_HPP
#include "engine/Entity.hpp"
#include <cstdint>
#include <map>

namespace server {

/**
 * @struct ConnectedPlayer
 * @brief What the game thread knows about a connected player.
 * @param ship The player's ship; it may have been destroyed since.
 * @param slot The player's slot, from 1 to MaxPlayers.
 * @param lastSequence The sequence of the last input applied, to drop
 * older inputs that arrive late.
 **/
struct ConnectedPlayer {
    engine::Entity ship{};
    std::uint8_t slot = 0;
    std::uint32_t lastSequence = 0;
};

/**
 * @brief The connected players of the game, by ClientId.
 **/
using Players = std::map<std::uint32_t, ConnectedPlayer>;

} // namespace server

#endif // SERVER_PLAYERS_HPP
