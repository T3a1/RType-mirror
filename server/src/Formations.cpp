/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Formations
*/

#include "Formations.hpp"
#include "EnemyCatalog.hpp"
#include "EnemyDefs.hpp"
#include "common/Playfield.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace server {

namespace {

/**
 * @brief Returns the stats of the enemies of a formation.
 * @param formation The formation.
 * @return The stats of its catalog kind, or of its built-in type.
 **/
const EnemyDef &statsOf(const Formation &formation)
{
    return formation.kind ? enemyCatalog().at(*formation.kind).def
                          : enemyDef(formation.type);
}

} // namespace

float formationExtent(const Formation &formation)
{
    const EnemyDef &stat = statsOf(formation);
    const rtype::MovementPattern pattern =
        formation.pattern.value_or(stat.pattern);
    const float rows = static_cast<float>(formation.count - 1) / 2.0F;

    return (rows * formation.stepY) + pattern.amplitude + (stat.size.y / 2.0F);
}

float randomCenterY(const Formation &formation, rtype::Spawner &spawner)
{
    const float extent =
        std::min(formationExtent(formation), rtype::PlayfieldHeight / 2.0F);
    std::uniform_real_distribution<float> center(
        extent, rtype::PlayfieldHeight - extent);

    return center(spawner.randomGenerator);
}

std::vector<PendingSpawn> formationSpawns(const Formation &formation,
                                          float centerY)
{
    const EnemyDef &stat = statsOf(formation);
    const float middle = static_cast<float>(formation.count - 1) / 2.0F;
    std::vector<PendingSpawn> spawns;

    spawns.reserve(formation.count);
    for (std::size_t i = 0; i < formation.count; ++i) {
        const float rank = static_cast<float>(i) - middle;
        const float turn =
            formation.fromCenter ? std::abs(rank) : static_cast<float>(i);
        spawns.push_back({
            .delay = formation.spacing * turn,
            .request =
                {
                    .type = formation.type,
                    .position =
                        {
                            .x = rtype::PlayfieldWidth + stat.size.x,
                            .y = centerY + (rank * formation.stepY),
                        },
                    .pattern = formation.pattern,
                    .kind = formation.kind,
                },
        });
    }
    return spawns;
}

} // namespace server
