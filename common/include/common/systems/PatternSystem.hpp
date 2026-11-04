/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PatternSystem
*/

#ifndef COMMON_SYSTEMS_PATTERNSYSTEM_HPP
#define COMMON_SYSTEMS_PATTERNSYSTEM_HPP

#include "common/components/MovementPattern.hpp"
#include "engine/Clock.hpp"
#include "engine/Registry.hpp"

namespace rtype {

/**
 * @brief Returns how far above or below baseY a Sine or Zigzag path is.
 * @param pattern The pattern.
 * @param elapsed The time since the pattern started.
 * @return The vertical offset from baseY, in playfield units (0 for
 * Straight and Dive).
 **/
float patternOffset(const MovementPattern &pattern, engine::Duration elapsed);

/**
 * @brief Advances every MovementPattern by dt and sets the Velocity of its
 * entity so that movementSystem() brings it exactly onto its path this
 * tick. The horizontal speed is kept, except during a dive. Runs right
 * before movementSystem().
 * @param registry The registry.
 * @param dt The duration of the tick.
 **/
void patternSystem(engine::Registry &registry, engine::Duration dt);

} // namespace rtype

#endif // COMMON_SYSTEMS_PATTERNSYSTEM_HPP
