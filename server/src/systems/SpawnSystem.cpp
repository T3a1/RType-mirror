/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnSystem
*/

#include "systems/SpawnSystem.hpp"

namespace server {

namespace {

constexpr float SPAWN_X = 1980.0F;
constexpr float MIN_Y = 60.0F;
constexpr float MAX_Y = 1020.0F;

/**
 * @brief Draws the delay before the next spawn, between the spawner's
 * minimum and maximum delays.
 * @param spawner The spawner to draw the delay for.
 * @return The delay, in seconds.
 **/
engine::Duration randomDelay(rtype::Spawner &spawner)
{
    std::uniform_real_distribution<float> delay(spawner.minDelay.count(),
                                                spawner.maxDelay.count());

    return engine::Duration{delay(spawner.randomGenerator)};
}

/**
 * @brief Draws an enemy of a random type at a random height, just right of
 * the playfield.
 * @param spawner The spawner whose random generator is used.
 * @return The spawn request.
 **/
SpawnEnemy randomRequest(rtype::Spawner &spawner)
{
    std::uniform_real_distribution<float> y(MIN_Y, MAX_Y);
    std::uniform_int_distribution<std::size_t> type(0, ENEMY_DEFS.size() - 1);

    return {
        .type = static_cast<rtype::EnemyType>(type(spawner.randomGenerator)),
        .position = {.x = SPAWN_X, .y = y(spawner.randomGenerator)},
    };
}

} // namespace

void spawnSystem(engine::Registry &registry, EntityFactory &factory,
                 engine::Duration dt)
{
    std::vector<SpawnEnemy> spawnRequests;

    registry.each<rtype::Spawner>([&](engine::Entity, rtype::Spawner &spawner) {
        spawner.timeUntilNextSpawn -= dt;
        while (spawner.timeUntilNextSpawn.count() <= 0.0F) {
            spawnRequests.push_back(randomRequest(spawner));
            spawner.timeUntilNextSpawn += randomDelay(spawner);
        }
    });
    for (const SpawnEnemy &request : spawnRequests) {
        factory.create(registry, request);
    }
}

} // namespace server
