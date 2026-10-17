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

struct ConnectedPlayer {
    engine::Entity ship{};
    std::uint8_t slot = 0;
    std::uint32_t lastSequence = 0;
};

using Players = std::map<std::uint32_t, ConnectedPlayer>;

} // namespace server

#endif // SERVER_PLAYERS_HPP
