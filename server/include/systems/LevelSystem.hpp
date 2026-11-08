/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** LevelSystem
*/

#ifndef SERVER_SYSTEMS_LEVELSYSTEM_HPP
#define SERVER_SYSTEMS_LEVELSYSTEM_HPP

#include "engine/Clock.hpp"
#include "engine/Registry.hpp"

#include <cstdint>
#include <optional>

namespace server {

/** @brief The pause between two levels. */
inline constexpr engine::Duration LevelPause{3.0F};

/**
 * @brief Plays the levels: queues each wave of the current level in the
 * SpawnQueue when its time comes. Once every wave of a level has spawned
 * and no enemy is left, its boss comes, if it has one; once the boss is
 * destroyed too, the next level starts after LevelPause. After the
 * last level, the LevelProgress is removed and spawnSystem() goes on with
 * random waves (endless mode). Runs before spawnSystem().
 * @param registry The registry holding the spawner (Spawner, SpawnQueue
 * and LevelProgress) and the enemies.
 * @param dt The time elapsed since the last tick.
 * @return The level announced this tick, for GameEvent{LevelStarted}: 1
 * on the first tick, the next level (from 2) as soon as one is cleared,
 * before its pause, or 0 when the endless mode follows the last level.
 * Nothing on the other ticks.
 **/
[[nodiscard]] std::optional<std::uint8_t>
levelSystem(engine::Registry &registry, engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_LEVELSYSTEM_HPP
