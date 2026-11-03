/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PatternSystem
*/

#include "common/systems/PatternSystem.hpp"
#include "common/components/Player.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "engine/Entity.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <optional>
#include <vector>

namespace rtype {

namespace {

/**
 * @brief Returns the position of the player closest to a point.
 * @param players The positions of the players.
 * @param from The point.
 * @return The closest position, or nothing if there is no player.
 **/
std::optional<engine::Vec2f>
nearestPlayer(const std::vector<engine::Vec2f> &players, engine::Vec2f from)
{
    std::optional<engine::Vec2f> nearest;
    float best = std::numeric_limits<float>::max();
    for (const engine::Vec2f &player : players) {
        const engine::Vec2f delta = player - from;
        const float distance = (delta.x * delta.x) + (delta.y * delta.y);
        if (distance < best) {
            best = distance;
            nearest = player;
        }
    }
    return nearest;
}

/**
 * @brief Returns the direction of a dive starting at a point: toward the
 * nearest player, or straight to the left if there is none or it is right
 * there.
 * @param players The positions of the players.
 * @param from The point where the dive starts.
 * @return A direction of length 1.
 **/
engine::Vec2f diveDirection(const std::vector<engine::Vec2f> &players,
                            engine::Vec2f from)
{
    const std::optional<engine::Vec2f> target = nearestPlayer(players, from);
    if (!target) {
        return {.x = -1.0F, .y = 0.0F};
    }
    const engine::Vec2f delta = *target - from;
    const float length = std::hypot(delta.x, delta.y);
    if (length <= 0.0F) {
        return {.x = -1.0F, .y = 0.0F};
    }
    return delta * (1.0F / length);
}

} // namespace

float patternOffset(const MovementPattern &pattern, engine::Duration elapsed)
{
    const float angle = (2.0F * std::numbers::pi_v<float> * pattern.frequency *
                         elapsed.count()) +
                        pattern.phase;
    switch (pattern.kind) {
    case MovementKind::Sine:
        return pattern.amplitude * std::sin(angle);
    case MovementKind::Zigzag:
        // asin(sin(x)) is a triangle wave between -pi/2 and pi/2.
        return pattern.amplitude * 2.0F / std::numbers::pi_v<float> *
               std::asin(std::clamp(std::sin(angle), -1.0F, 1.0F));
    case MovementKind::Straight:
    case MovementKind::Dive:
        return 0.0F;
    }
    return 0.0F;
}

void patternSystem(engine::Registry &registry, engine::Duration dt)
{
    if (dt.count() <= 0.0F) {
        return;
    }
    std::vector<engine::Vec2f> players;
    registry.each<Player, Transform>(
        [&players](engine::Entity, const Player &, const Transform &transform) {
            players.push_back(transform.position);
        });

    registry.each<MovementPattern, Transform, Velocity>(
        [&players, dt](engine::Entity, MovementPattern &pattern,
                       const Transform &transform, Velocity &velocity) {
            pattern.elapsed += dt;
            if (pattern.kind == MovementKind::Dive &&
                pattern.elapsed >= pattern.diveDelay) {
                if (pattern.diveDirection == engine::Vec2f{}) {
                    pattern.diveDirection =
                        diveDirection(players, transform.position);
                }
                velocity.value = pattern.diveDirection * pattern.diveSpeed;
                return;
            }
            const float target =
                pattern.baseY + patternOffset(pattern, pattern.elapsed);
            velocity.value.y = (target - transform.position.y) / dt.count();
        });
}

} // namespace rtype
