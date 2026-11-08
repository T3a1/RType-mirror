/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PredictionSystem
*/

#ifndef COMMON_SYSTEMS_PREDICTIONSYSTEM_HPP
#define COMMON_SYSTEMS_PREDICTIONSYSTEM_HPP

#include "common/components/Prediction.hpp"
#include "engine/Clock.hpp"
#include "engine/Vec2.hpp"

#include <cstdint>
#include <optional>

namespace rtype {

/**
 * @brief How fast a correction of the prediction fades out, per second: about
 *95 % of it is absorbed in 0.3 s.
 **/
inline constexpr float CorrectionFadeRate = 10.0F;

/**
 * @brief A correction larger than this, in playfield units, is applied at once
 * instead of being smoothed (the ship was teleported).
 **/
inline constexpr float MaxSmoothedCorrection = 150.0F;

/**
 * @brief Moves a player ship like the server does: shipVelocity() for a
 * duration, then kept inside the playfield.
 * @param position The ship's position.
 * @param actions The held actions, as Action bits.
 * @param duration How long they are held, in seconds.
 * @return The new position.
 **/
engine::Vec2f moveShip(engine::Vec2f position, std::uint8_t actions,
                       float duration);

/**
 * @brief Records the input of one frame. When more than
 * Prediction::MaxPending inputs are pending, the oldest is dropped.
 * @param prediction The prediction.
 * @param sequence The sequence number of the Input that carries this frame,
 * sent now or later; 0 (not connected) is ignored.
 * @param actions The held actions.
 * @param dt How long the client applies it: the duration of the frame.
 **/
void recordInput(Prediction &prediction, std::uint32_t sequence,
                 std::uint8_t actions, engine::Duration dt);

/**
 * @brief Applies a snapshot: the ship's position on the server and the last
 * input the server applied, both at the same tick. Drops the inputs up to
 * that one.
 * @param prediction The prediction.
 * @param lastInput The sequence of the last input applied by the server.
 * @param position The ship's position in the snapshot.
 **/
void acknowledgeInputs(Prediction &prediction, std::uint32_t lastInput,
                       engine::Vec2f position);

/**
 * @brief Predicts the ship's position: the server's position with every
 * pending input replayed on top. A difference with the position predicted
 * at the last frame (the server disagreed) goes into the offset, which fades
 * out at CorrectionFadeRate, so the ship never jumps.
 * @param prediction The prediction, updated.
 * @param dt The duration of the frame.
 * @return The position to draw, none before the first snapshot.
 **/
std::optional<engine::Vec2f> predictShip(Prediction &prediction,
                                         engine::Duration dt);

} // namespace rtype

#endif // COMMON_SYSTEMS_PREDICTIONSYSTEM_HPP
