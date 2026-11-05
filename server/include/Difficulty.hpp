/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Difficulty
*/

#ifndef SERVER_DIFFICULTY_HPP
#define SERVER_DIFFICULTY_HPP

#include "common/protocol/Messages.hpp"

namespace server {

/**
 * @struct DifficultyStats
 * @brief How a difficulty changes the enemies, as factors of their stats.
 * @param enemyHealth Factor of the health of every enemy, bosses included.
 * @param enemyFireInterval Factor of the time between two enemy shots:
 * below 1, they shoot more often.
 * @param spawnDelay Factor of the delay between two random waves of the
 * endless mode.
 **/
struct DifficultyStats {
    float enemyHealth = 1.0F;
    float enemyFireInterval = 1.0F;
    float spawnDelay = 1.0F;
};

/**
 * @brief Returns how a difficulty changes the enemies.
 * @param difficulty The difficulty chosen for the game.
 * @return Its factors; all 1 for Normal.
 **/
constexpr DifficultyStats
difficultyStats(rtype::protocol::Difficulty difficulty)
{
    switch (difficulty) {
    case rtype::protocol::Difficulty::Easy:
        return {
            .enemyHealth = 0.6F, .enemyFireInterval = 1.5F, .spawnDelay = 1.4F};
    case rtype::protocol::Difficulty::Normal:
        return {};
    case rtype::protocol::Difficulty::Hard:
        return {
            .enemyHealth = 1.5F, .enemyFireInterval = 0.7F, .spawnDelay = 0.7F};
    }
    return {};
}

} // namespace server

#endif // SERVER_DIFFICULTY_HPP
