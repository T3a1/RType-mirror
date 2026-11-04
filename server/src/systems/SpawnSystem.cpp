/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SpawnSystem
*/

#include "systems/SpawnSystem.hpp"
#include "common/Playfield.hpp"

namespace server {

namespace {

/**
 * @brief Draws the delay before the next formation, between the spawner's
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
 * @brief Draws a random formation at a random height that keeps it inside
 * the playfield.
 * @param spawner The spawner whose random generator is used.
 * @return The enemies of the formation.
 **/
std::vector<PendingSpawn> randomFormation(rtype::Spawner &spawner)
{
    std::uniform_int_distribution<std::size_t> index(0, FORMATIONS.size() - 1);
    const Formation &formation = FORMATIONS.at(index(spawner.randomGenerator));
    const float extent = formationExtent(formation);
    std::uniform_real_distribution<float> center(
        extent, rtype::PlayfieldHeight - extent);

    return formationSpawns(formation, center(spawner.randomGenerator));
}

} // namespace

void spawnSystem(engine::Registry &registry, EntityFactory &factory,
                 engine::Duration dt)
{
    std::vector<SpawnEnemy> spawnRequests;

    registry.each<rtype::Spawner, SpawnQueue>(
        [&](engine::Entity, rtype::Spawner &spawner, SpawnQueue &queue) {
            spawner.timeUntilNextSpawn -= dt;
            while (spawner.timeUntilNextSpawn.count() <= 0.0F) {
                const std::vector<PendingSpawn> spawns =
                    randomFormation(spawner);
                queue.pending.insert(queue.pending.end(), spawns.begin(),
                                     spawns.end());
                spawner.timeUntilNextSpawn += randomDelay(spawner);
            }
            for (PendingSpawn &spawn : queue.pending) {
                spawn.delay -= dt;
                if (spawn.delay.count() <= 0.0F) {
                    spawnRequests.push_back(spawn.request);
                }
            }
            std::erase_if(queue.pending, [](const PendingSpawn &spawn) {
                return spawn.delay.count() <= 0.0F;
            });
        });
    for (const SpawnEnemy &request : spawnRequests) {
        factory.create(registry, request);
    }
}

} // namespace server
