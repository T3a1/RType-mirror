/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Animation
*/

#ifndef CLIENT_COMPONENTS_ANIMATION_HPP
#define CLIENT_COMPONENTS_ANIMATION_HPP

#include "graphics/Types.hpp"

#include <span>

namespace client {

/**
 * @struct AnimationClip
 * @brief A sequence of frames of one sprite sheet.
 * @param frames The source rectangle of each frame, all of the same size.
 * @param frameTime How long each frame is shown, in seconds.
 * @param loop Whether it starts over at the end; if not, the entity is
 * destroyed after its last frame (explosions and other effects).
 **/
struct AnimationClip {
    std::span<const gfx::Rect> frames;
    float frameTime = 0.1F;
    bool loop = true;
};

/**
 * @struct Animation
 * @brief Plays a clip on the Sprite of an entity.
 * @param clip The clip played, never null. It is static data: the
 * component only points to it.
 * @param elapsed The time since the clip started, in seconds.
 **/
struct Animation {
    const AnimationClip *clip = nullptr;
    float elapsed = 0.0F;
};

/**
 * @struct ShipTilt
 * @brief Tilts a player ship's sprite while it moves up or down.
 * @param frames The ship's five frames, from fully tilted down to fully
 * tilted up.
 * @param previousY Its vertical position at its last move.
 * @param direction The direction of its last move: 1 up, -1 down.
 * @param moving How long it still counts as moving, in seconds.
 * @param lean Its tilt, from -1 (down) to 1 (up); it follows the movement
 * smoothly.
 **/
struct ShipTilt {
    std::span<const gfx::Rect> frames;
    float previousY = 0.0F;
    float direction = 0.0F;
    float moving = 0.0F;
    float lean = 0.0F;
};

/**
 * @struct Facing
 * @brief Plays the clip of the direction an entity walks in: left or right.
 * @param left The clip played while it moves left, never null.
 * @param right The clip played while it moves right, never null.
 * @param previousX Its horizontal position at its last move.
 **/
struct Facing {
    const AnimationClip *left = nullptr;
    const AnimationClip *right = nullptr;
    float previousX = 0.0F;
};

} // namespace client

#endif // CLIENT_COMPONENTS_ANIMATION_HPP
