/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** GameOverSystem
*/

#ifndef SERVER_SYSTEMS_GAMEOVERSYSTEM_HPP
#define SERVER_SYSTEMS_GAMEOVERSYSTEM_HPP

#include "Players.hpp"
#include "engine/Registry.hpp"
#include "network/NetworkMessages.hpp"

namespace server {

/**
 * @brief Keeps the score of every player with a ship, and tells the clients
 * once when no connected player has a ship left. A later player's ship starts
 * a new game over the same world. Runs after the damage and the deaths of the
 * tick, before the registry is flushed, so a ship dying this tick still holds
 * its score.
 * @param registry The registry holding the ships.
 * @param players The connected players, whose scores are updated.
 * @param outbound The queue GameOver is pushed to.
 * @param gameOver Whether GameOver was already sent for this game, updated.
 **/
void gameOverSystem(engine::Registry &registry, Players &players,
                    Outbound &outbound, bool &gameOver);

} // namespace server

#endif // SERVER_SYSTEMS_GAMEOVERSYSTEM_HPP
