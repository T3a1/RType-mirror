/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Prediction
*/

#ifndef COMMON_COMPONENTS_PREDICTION_HPP
#define COMMON_COMPONENTS_PREDICTION_HPP

#include "engine/Vec2.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>

namespace rtype {

/**
 * @struct PendingInput
 * @brief The input of one frame, not yet acknowledged by a snapshot.
 * @param sequence The sequence number of the Input that carries this frame.
 * @param actions The held actions, as Action bits.
 * @param duration How long the client applied it, in seconds.
 **/
struct PendingInput {
    std::uint32_t sequence = 0;
    std::uint8_t actions = 0;
    float duration = 0.0F;
};

/**
 * @struct Prediction
 * @brief The client-side prediction of the local ship: the last position
 * the server sent, the inputs it has not applied yet, and the smoothing of
 * the corrections.
 * @param MaxPending How many inputs are kept at most.
 * @param pending The unacknowledged inputs, oldest first, at most
 * MaxPending.
 * @param authoritative The ship's position in the newest snapshot, none
 * before the first one.
 * @param acknowledged The sequence of the last input the server applied in
 * that snapshot.
 * @param predicted The position predicted at the last frame.
 * @param offset What is left to absorb of the past corrections, added to
 * the drawn position and fading out.
 * @param lastCorrection The size of the correction at the last frame, in
 * playfield units.
 * @param hasNewInput Whether an input was recorded since the last frame.
 **/
struct Prediction {
    static constexpr std::size_t MaxPending = 1024;
    std::deque<PendingInput> pending;
    std::optional<engine::Vec2f> authoritative;
    std::uint32_t acknowledged = 0;
    std::optional<engine::Vec2f> predicted;
    engine::Vec2f offset;
    float lastCorrection = 0.0F;
    bool hasNewInput = false;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_PREDICTION_HPP
