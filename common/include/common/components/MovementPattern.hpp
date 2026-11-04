/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** MovementPattern
*/

#ifndef COMMON_COMPONENTS_MOVEMENTPATTERN_HPP
#define COMMON_COMPONENTS_MOVEMENTPATTERN_HPP

#include "engine/Clock.hpp"
#include "engine/Vec2.hpp"

#include <cstdint>

namespace rtype {

/**
 * @enum MovementKind
 * @brief The path an entity follows while its Velocity moves it forward.
 * @param Straight Stays at baseY.
 * @param Sine Oscillates smoothly around baseY.
 * @param Zigzag Goes up and down around baseY in straight segments.
 * @param Dive Stays at baseY for diveDelay, then rushes in a straight line
 * toward where its diveTarget was at that moment.
 **/
enum class MovementKind : std::uint8_t {
    Straight,
    Sine,
    Zigzag,
    Dive,
};

/**
 * @enum DiveTarget
 * @brief What a Dive rushes toward.
 * @param NearestPlayer The closest player ship (enemies, enemy missiles).
 * @param NearestEnemy The closest enemy (player missiles).
 **/
enum class DiveTarget : std::uint8_t {
    NearestPlayer,
    NearestEnemy,
};

/**
 * @struct MovementPattern
 * @brief Steers the vertical movement of an entity (and its whole movement
 * once it dives), by setting its Velocity before movementSystem() moves it.
 * Works on any entity with a Transform and a Velocity: enemies as well as
 * missiles. The horizontal speed is the one the entity was created with.
 * @param kind The path followed.
 * @param baseY The height of the middle of the path, in playfield units.
 * Set on the first tick, so that the path starts where the entity spawned:
 * the creator does not fill it.
 * @param amplitude How far the path goes above and below baseY, in
 * playfield units (Sine, Zigzag).
 * @param frequency The number of full up-and-down cycles per second (Sine,
 * Zigzag).
 * @param phase Where in its cycle the path starts, in radians (Sine,
 * Zigzag): entities of one formation get different phases.
 * @param diveDelay How long the entity flies straight before diving (Dive).
 * @param diveSpeed Its speed once it dives, in playfield units per second
 * (Dive).
 * @param diveTarget What it rushes toward (Dive). With no target left, it
 * keeps going in the direction it was flying.
 * @param elapsed The time since the pattern started.
 * @param diveDirection The unit direction of the dive, (0, 0) until it
 * starts.
 * @param anchored Whether baseY was set.
 **/
struct MovementPattern {
    MovementKind kind = MovementKind::Straight;
    float baseY = 0.0F;
    float amplitude = 0.0F;
    float frequency = 0.0F;
    float phase = 0.0F;
    engine::Duration diveDelay{0.0F};
    float diveSpeed = 0.0F;
    DiveTarget diveTarget = DiveTarget::NearestPlayer;
    engine::Duration elapsed{0.0F};
    engine::Vec2f diveDirection;
    bool anchored = false;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_MOVEMENTPATTERN_HPP
