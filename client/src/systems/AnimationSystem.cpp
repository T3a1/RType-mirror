/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** AnimationSystem
*/

#include "systems/AnimationSystem.hpp"
#include "common/components/Transform.hpp"
#include "components/Animation.hpp"
#include "components/Sprite.hpp"
#include "engine/Entity.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace client {

namespace {

/** @brief A ship counts as moving while its position changed in the last
 * MoveMemory seconds: the local ship only moves when a snapshot arrives (20 per
 * second), not every frame. */
constexpr float MoveMemory = 0.12F;
/** @brief Vertical moves smaller than this, in playfield units, are ignored. */
constexpr float MoveThreshold = 0.5F;
/** @brief How fast the tilt follows the movement, in full tilts per second. */
constexpr float TiltSpeed = 6.0F;

} // namespace

void animationSystem(engine::Registry &registry, engine::Duration dt)
{
    registry.each<Animation, Sprite>([&registry, dt](engine::Entity entity,
                                                     Animation &animation,
                                                     Sprite &sprite) {
        const AnimationClip &clip = *animation.clip;
        if (clip.frames.empty() || clip.frameTime <= 0.0F) {
            return;
        }
        const float duration =
            clip.frameTime * static_cast<float>(clip.frames.size());
        animation.elapsed += dt.count();
        if (animation.elapsed >= duration) {
            if (!clip.loop) {
                registry.destroy(entity);
                return;
            }
            animation.elapsed = std::fmod(animation.elapsed, duration);
        }
        const auto index = std::min(
            static_cast<std::size_t>(animation.elapsed / clip.frameTime),
            clip.frames.size() - 1);
        sprite.source = clip.frames[index];
    });
}

void shipTiltSystem(engine::Registry &registry, engine::Duration dt)
{
    const float seconds = dt.count();
    registry.each<ShipTilt, rtype::Transform, Sprite>(
        [seconds](engine::Entity, ShipTilt &tilt,
                  const rtype::Transform &transform, Sprite &sprite) {
            if (tilt.frames.empty()) {
                return;
            }
            const float move = transform.position.y - tilt.previousY;
            if (std::abs(move) > MoveThreshold) {
                tilt.previousY = transform.position.y;
                tilt.direction = move < 0.0F ? 1.0F : -1.0F; // y points down
                tilt.moving = MoveMemory;
            } else {
                tilt.moving = std::max(tilt.moving - seconds, 0.0F);
            }
            const float target = tilt.moving > 0.0F ? tilt.direction : 0.0F;
            const float step = TiltSpeed * seconds;
            tilt.lean += std::clamp(target - tilt.lean, -step, step);

            const std::size_t level = tilt.frames.size() / 2;
            const auto middle = static_cast<float>(level);
            const auto index = static_cast<std::size_t>(
                std::lround(middle + (tilt.lean * middle)));
            sprite.source =
                tilt.frames[std::min(index, tilt.frames.size() - 1)];
        });
}

void facingSystem(engine::Registry &registry, engine::Duration /*dt*/)
{
    registry.each<Facing, Animation, rtype::Transform>(
        [](engine::Entity, Facing &facing, Animation &animation,
           const rtype::Transform &transform) {
            const float move = transform.position.x - facing.previousX;
            if (std::abs(move) <= MoveThreshold) {
                return;
            }
            facing.previousX = transform.position.x;
            animation.clip = move < 0.0F ? facing.left : facing.right;
        });
}

} // namespace client
