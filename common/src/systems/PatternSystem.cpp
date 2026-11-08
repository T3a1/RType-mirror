/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PatternSystem
*/

#include "common/systems/PatternSystem.hpp"
#include "common/components/Enemy.hpp"
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
 * @struct Target
 * @brief An entity a dive can rush toward.
 * @param entity The entity.
 * @param position Its position.
 **/
struct Target {
    engine::Entity entity;
    engine::Vec2f position;
};

/**
 * @brief Returns the positions of every entity with a C and a Transform.
 * @param registry The registry.
 * @return The targets.
 **/
template <typename C> std::vector<Target> targets(engine::Registry &registry)
{
    std::vector<Target> found;
    registry.each<C, Transform>(
        [&found](engine::Entity entity, const C &, const Transform &transform) {
            found.push_back({.entity = entity, .position = transform.position});
        });
    return found;
}

/**
 * @brief Returns the unit vector of a direction, or nothing if it is null.
 * @param direction The direction.
 * @return The direction with a length of 1.
 **/
std::optional<engine::Vec2f> normalized(engine::Vec2f direction)
{
    const float length = std::hypot(direction.x, direction.y);
    if (length <= 0.0F) {
        return std::nullopt;
    }
    return direction * (1.0F / length);
}

/**
 * @brief Returns the direction of a dive: toward the closest target other
 * than the diving entity, or on along its current velocity if there is
 * none (straight to the left if it is not moving).
 * @param candidates The possible targets.
 * @param self The diving entity.
 * @param from Where the dive starts.
 * @param velocity The current velocity of the diving entity.
 * @return A direction of length 1.
 **/
engine::Vec2f diveDirection(const std::vector<Target> &candidates,
                            engine::Entity self, engine::Vec2f from,
                            engine::Vec2f velocity)
{
    std::optional<engine::Vec2f> nearest;
    float best = std::numeric_limits<float>::max();
    for (const Target &target : candidates) {
        const engine::Vec2f delta = target.position - from;
        const float distance = (delta.x * delta.x) + (delta.y * delta.y);
        if (target.entity != self && distance < best) {
            best = distance;
            nearest = delta;
        }
    }
    if (nearest) {
        if (const auto direction = normalized(*nearest)) {
            return *direction;
        }
    }
    return normalized(velocity).value_or(engine::Vec2f{.x = -1.0F, .y = 0.0F});
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
    const std::vector<Target> players = targets<Player>(registry);
    const std::vector<Target> enemies = targets<Enemy>(registry);

    registry.each<MovementPattern, Transform, Velocity>(
        [&players, &enemies,
         dt](engine::Entity entity, MovementPattern &pattern,
             const Transform &transform, Velocity &velocity) {
            if (!pattern.anchored) {
                pattern.baseY = transform.position.y -
                                patternOffset(pattern, pattern.elapsed);
                pattern.anchored = true;
            }
            pattern.elapsed += dt;
            if (pattern.kind == MovementKind::Dive &&
                pattern.elapsed >= pattern.diveDelay) {
                if (pattern.diveDirection == engine::Vec2f{}) {
                    pattern.diveDirection = diveDirection(
                        pattern.diveTarget == DiveTarget::NearestEnemy
                            ? enemies
                            : players,
                        entity, transform.position, velocity.value);
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
