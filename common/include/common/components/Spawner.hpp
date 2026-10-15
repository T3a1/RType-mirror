/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Spawner
*/

#ifndef COMMON_COMPONENTS_SPAWNER_HPP
#define COMMON_COMPONENTS_SPAWNER_HPP

#include <random>

#include "engine/Clock.hpp"

namespace rtype {

/**
 * A component that spawns a random enemy after a random delay between minDelay
 *and maxDelay.
 * @param minDelay The minimum delay between spawns.
 * @param maxDelay The maximum delay between spawns.
 * @param timeUntilNextSpawn The time left until the next spawn.
 * @param randomGenerator A random number generator used to determine the next
 * spawn delay.
 **/
struct Spawner {

    engine::Duration minDelay{0.5F};
    engine::Duration maxDelay{2.0F};
    engine::Duration timeUntilNextSpawn{1.0F};
    std::mt19937 randomGenerator{std::random_device{}()};
};

} // namespace rtype

#endif // COMMON_COMPONENTS_SPAWNER_HPP
