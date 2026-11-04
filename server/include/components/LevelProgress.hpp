/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** LevelProgress
*/

#ifndef SERVER_COMPONENTS_LEVELPROGRESS_HPP
#define SERVER_COMPONENTS_LEVELPROGRESS_HPP

#include "Level.hpp"
#include "engine/Clock.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace server {

/**
 * @struct LevelProgress
 * @brief Where a game stands in its levels. While the spawner entity has
 * it, the waves come from the levels instead of being random.
 * @param levels The levels played one after the other, shared by every
 * room and never changed.
 * @param level The index of the current level.
 * @param nextWave The index of the next wave of the current level.
 * @param elapsed The time since the current level started.
 * @param pause The time left before the current level starts.
 **/
struct LevelProgress {
    std::shared_ptr<const std::vector<Level>> levels;
    std::size_t level = 0;
    std::size_t nextWave = 0;
    engine::Duration elapsed{0.0F};
    engine::Duration pause{0.0F};
};

} // namespace server

#endif // SERVER_COMPONENTS_LEVELPROGRESS_HPP
