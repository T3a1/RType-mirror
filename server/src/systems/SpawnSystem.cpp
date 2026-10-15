/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnSystem
*/

#include "systems/SpawnSystem.hpp"

namespace server {

constexpr float SPAWN_X = 1980.0F;
constexpr float MIN_Y = 60.0F;
constexpr float MAX_Y = 1020.0F;

/**
 * Generates a random delay for the next spawn.
 * @param spawner The spawner to generate the delay for.
 * @return A random delay.
 **/
engine::Duration randomDelay(rtype::Spawner &spawner)
{
    std::uniform_real_distribution<float> delay(spawner.minDelay.count(),
                                                spawner.maxDelay.count());

    return engine::Duration{delay(spawner.randomGenerator)};
}

/**
 * Generates a random enemy spawn request.
 * @param spawner The spawner to generate the request for.
 * @return A random spawn request.
 **/
SpawnEnemy randomRequest(rtype::Spawner &spawner)
{
    std::uniform_real_distribution<float> y(MIN_Y, MAX_Y);

    return {.type = rtype::EnemyType::Basic,
            .position = {.x = SPAWN_X, .y = y(spawner.randomGenerator)}};
}

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
