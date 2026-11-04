/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** LevelSystem
*/

#include "systems/LevelSystem.hpp"
#include "Formations.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Spawner.hpp"
#include "components/LevelProgress.hpp"
#include "components/SpawnQueue.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace server {

namespace {

/** @brief Whether an enemy is still alive. */
bool anyEnemy(engine::Registry &registry)
{
    bool found = false;
    registry.each<rtype::Enemy>(
        [&found](engine::Entity, const rtype::Enemy &) { found = true; });
    return found;
}

/**
 * @brief Queues the waves of the current level whose time has come.
 * @param progress The level progress.
 * @param spawner The spawner, for the random heights.
 * @param queue The queue the enemies go to.
 **/
void queueDueWaves(LevelProgress &progress, rtype::Spawner &spawner,
                   SpawnQueue &queue)
{
    const Level &level = progress.levels->at(progress.level);
    while (progress.nextWave < level.waves.size() &&
           level.waves.at(progress.nextWave).time <= progress.elapsed) {
        const Wave &wave = level.waves.at(progress.nextWave);
        const float centerY =
            wave.centerY.value_or(randomCenterY(wave.formation, spawner));
        const std::vector<PendingSpawn> spawns =
            formationSpawns(wave.formation, centerY);
        queue.pending.insert(queue.pending.end(), spawns.begin(), spawns.end());
        ++progress.nextWave;
    }
}

/** @brief The level number sent to the clients, from 1, capped at 255. */
std::uint8_t levelNumber(std::size_t index)
{
    return static_cast<std::uint8_t>(std::min<std::size_t>(
        index + 1, std::numeric_limits<std::uint8_t>::max()));
}

} // namespace

std::optional<std::uint8_t> levelSystem(engine::Registry &registry,
                                        engine::Duration dt)
{
    const bool enemiesLeft = anyEnemy(registry);
    std::vector<engine::Entity> finished;
    std::optional<std::uint8_t> announced;

    registry.each<LevelProgress, rtype::Spawner, SpawnQueue>(
        [&](engine::Entity entity, LevelProgress &progress,
            rtype::Spawner &spawner, SpawnQueue &queue) {
            if (!progress.levels || progress.level >= progress.levels->size()) {
                finished.push_back(entity);
                return;
            }
            if (progress.pause.count() > 0.0F) {
                progress.pause -= dt;
                if (progress.pause.count() <= 0.0F) {
                    BB_INFO("Level "
                            << progress.level + 1 << ": "
                            << progress.levels->at(progress.level).name);
                }
                return;
            }
            if (progress.elapsed.count() == 0.0F && progress.nextWave == 0 &&
                progress.level == 0) {
                BB_INFO("Level 1: " << progress.levels->front().name);
                announced = levelNumber(0);
            }
            progress.elapsed += dt;
            queueDueWaves(progress, spawner, queue);
            const Level &level = progress.levels->at(progress.level);
            if (progress.nextWave < level.waves.size() ||
                !queue.pending.empty() || enemiesLeft) {
                return;
            }
            ++progress.level;
            progress.nextWave = 0;
            progress.elapsed = engine::Duration{0.0F};
            progress.pause = LevelPause;
            if (progress.level >= progress.levels->size()) {
                BB_INFO("Every level cleared: endless mode");
                finished.push_back(entity);
                announced = 0;
            } else {
                announced = levelNumber(progress.level);
            }
        });
    for (const engine::Entity entity : finished) {
        registry.remove<LevelProgress>(entity);
    }
    return announced;
}

} // namespace server
