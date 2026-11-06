/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** ObstacleSystem
*/

#include "systems/ObstacleSystem.hpp"
#include "common/Obstacles.hpp"
#include "common/Playfield.hpp"
#include "components/LevelProgress.hpp"
#include "engine/Entity.hpp"

namespace server {

namespace {

/**
 * @brief Queues one SpawnObstacle per block of a column, top first.
 * @param column The column to bring in.
 * @param commands Where the blocks go.
 **/
void queueColumn(const ObstacleColumn &column, SpawnCommands &commands)
{
    const float x = rtype::PlayfieldWidth + (rtype::TileSize.x / 2.0F);
    for (std::size_t row = 0; row < column.rows; ++row) {
        commands.emplace_back(SpawnObstacle{
            .position = {.x = x,
                         .y = column.top +
                              (static_cast<float>(row) * rtype::TileSize.y)},
            .destroyable = column.destroyable,
        });
    }
}

} // namespace

void obstacleSystem(engine::Registry &registry, SpawnCommands &commands)
{
    registry.each<LevelProgress>([&](engine::Entity, LevelProgress &progress) {
        if (!progress.levels || progress.level >= progress.levels->size() ||
            progress.pause.count() > 0.0F) {
            return;
        }
        const Level &level = progress.levels->at(progress.level);
        while (progress.nextObstacle < level.obstacles.size() &&
               level.obstacles.at(progress.nextObstacle).time <=
                   progress.elapsed) {
            queueColumn(level.obstacles.at(progress.nextObstacle), commands);
            ++progress.nextObstacle;
        }
    });
}

} // namespace server
