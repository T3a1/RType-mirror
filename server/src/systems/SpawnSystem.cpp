/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnSystem
*/

#include "systems/SpawnSystem.hpp"
#include "engine/Logger.hpp"

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
    std::uniform_real_distribution<float> delay(spawner.minDelay.count(), spawner.maxDelay.count());

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

/**
* Spawns an enemy based on a spawn request.
* @param registry The registry to spawn the enemy in.
* @param request The spawn request containing the enemy type and position.
**/
void spawnEnemy(engine::Registry &registry, const SpawnEnemy &request)
{
    const EnemyDef &def = enemyDef(request.type);
    const engine::Entity enemy = registry.create();

    registry.emplace<rtype::Transform>(enemy, rtype::Transform{.position = request.position});
    registry.emplace<rtype::Velocity>(enemy, rtype::Velocity{.value = {.x = -def.speed, .y = 0.0F}});
    registry.emplace<rtype::Health>(enemy, rtype::Health{.current = def.health, .max = def.health});
    registry.emplace<rtype::Damage>(enemy);
    registry.emplace<rtype::Enemy>(enemy, rtype::Enemy{.type = request.type});
    registry.emplace<rtype::EntityType>(enemy, def.entityType);

    registry.emplace<rtype::Collider>(enemy,
        rtype::Collider{.size = def.size,
                        .layer = rtype::CollisionLayer::Enemy,
                        .mask = rtype::CollisionLayer::Player | rtype::CollisionLayer::PlayerProjectile});

    BB_INFO("Enemy " << static_cast<int>(request.type) << " spawned at y=" << request.position.y);
}


void spawnSystem(engine::Registry &registry, engine::Duration dt)
{
    std::vector<SpawnEnemy> spawnRequests;

    registry.each<rtype::Spawner>(
        [&](engine::Entity, rtype::Spawner &spawner) {
            spawner.timeUntilNextSpawn -= dt;
            while (spawner.timeUntilNextSpawn.count() <= 0.0F) {
                spawnRequests.push_back(randomRequest(spawner));
                spawner.timeUntilNextSpawn += randomDelay(spawner);
            }
        });
    for (const SpawnEnemy &request : spawnRequests) {
        spawnEnemy(registry, request);
    }
}

} // namespace server
