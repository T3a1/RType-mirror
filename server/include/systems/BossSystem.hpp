/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BossSystem
*/

#ifndef SERVER_SYSTEMS_BOSSSYSTEM_HPP
#define SERVER_SYSTEMS_BOSSSYSTEM_HPP

#include "SpawnRequests.hpp"
#include "engine/Clock.hpp"
#include "engine/Registry.hpp"

#include <cstdint>

namespace server {

/** @brief The number of attack phases of a boss. */
inline constexpr std::uint8_t BossPhases = 3;

/**
 * @brief Returns the attack phase of a boss from its health: 1 above two
 * thirds, 2 above one third, 3 below.
 * @param health Its health points.
 * @param maxHealth Its health points when full.
 * @return The phase, from 1 to BossPhases.
 **/
[[nodiscard]] std::uint8_t bossPhase(int health, int maxHealth);

/**
 * @brief Drives the bosses. A boss comes in from the right and stops near
 * the right edge, then moves up and down and attacks, harder in each phase:
 * aimed shots at the nearest player (phase 1), a spread of 3 shots plus
 * an aimed one (phase 2), a fan of 5 shots, faster, while moving faster
 * (phase 3). Runs after patternSystem() and before movementSystem().
 * @param registry The registry holding the bosses and the players.
 * @param commands Where the shots are requested.
 * @param dt The time elapsed since the last tick.
 **/
void bossSystem(engine::Registry &registry, SpawnCommands &commands,
                engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_BOSSSYSTEM_HPP
