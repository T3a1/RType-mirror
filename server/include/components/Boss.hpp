/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Boss
*/

#ifndef SERVER_COMPONENTS_BOSS_HPP
#define SERVER_COMPONENTS_BOSS_HPP

#include "engine/Clock.hpp"

#include <cstdint>

namespace server {

/**
 * @struct Boss
 * @brief State of an end-level boss, driven by bossSystem().
 * @param phase Its attack phase, from 1 to BossPhases: it changes as its
 * health drops.
 * @param arrived Whether it reached its fighting position, after which it
 * moves up and down and attacks.
 * @param direction Its vertical direction once arrived: 1 down, -1 up.
 * @param cooldown The time left before its next attack.
 * @param walk Its horizontal direction once arrived, for a boss that walks:
 * 1 right, -1 left.
 * @param spin The angle of its rotating attacks, in radians.
 * @param volleys The number of attacks it made, to alternate them.
 **/
struct Boss {
    std::uint8_t phase = 1;
    bool arrived = false;
    float direction = 1.0F;
    engine::Duration cooldown{1.0F};
    float walk = -1.0F;
    float spin = 0.0F;
    std::uint32_t volleys = 0;
};

} // namespace server

#endif // SERVER_COMPONENTS_BOSS_HPP
