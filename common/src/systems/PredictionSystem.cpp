/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** PredictionSystem
*/

#include "common/systems/PredictionSystem.hpp"
#include "common/Playfield.hpp"
#include "common/systems/PlayerControlSystem.hpp"

#include <cmath>

namespace rtype {

engine::Vec2f moveShip(engine::Vec2f position, std::uint8_t actions,
                       float duration)
{
    return keepInPlayfield(position + (shipVelocity(actions) * duration),
                           PlayerShipSize);
}

void recordInput(Prediction &prediction, std::uint32_t sequence,
                 std::uint8_t actions, engine::Duration dt)
{
    if (sequence == 0) {
        return;
    }
    if (prediction.pending.size() == Prediction::MaxPending) {
        prediction.pending.pop_front();
    }
    prediction.pending.push_back({
        .sequence = sequence,
        .actions = actions,
        .duration = dt.count(),
    });
    prediction.hasNewInput = true;
}

void acknowledgeInputs(Prediction &prediction, std::uint32_t lastInput,
                       engine::Vec2f position)
{
    if (prediction.authoritative && lastInput < prediction.acknowledged) {
        return;
    }
    prediction.authoritative = position;
    prediction.acknowledged = lastInput;
    while (!prediction.pending.empty() &&
           prediction.pending.front().sequence <= lastInput) {
        prediction.pending.pop_front();
    }
}

std::optional<engine::Vec2f> predictShip(Prediction &prediction,
                                         engine::Duration dt)
{
    if (!prediction.authoritative) {
        return std::nullopt;
    }
    engine::Vec2f fresh = *prediction.authoritative;
    for (const PendingInput &input : prediction.pending) {
        fresh = moveShip(fresh, input.actions, input.duration);
    }
    prediction.lastCorrection = 0.0F;
    if (prediction.predicted) {
        engine::Vec2f expected = *prediction.predicted;
        if (prediction.hasNewInput && !prediction.pending.empty()) {
            const PendingInput &newest = prediction.pending.back();
            expected = moveShip(expected, newest.actions, newest.duration);
        }
        const engine::Vec2f error = expected - fresh;
        prediction.lastCorrection = std::hypot(error.x, error.y);
        prediction.offset += error;
        if (std::hypot(prediction.offset.x, prediction.offset.y) >
            MaxSmoothedCorrection) {
            prediction.offset = {};
        }
    }
    prediction.hasNewInput = false;
    prediction.offset *= std::exp(-CorrectionFadeRate * dt.count());
    prediction.predicted = fresh;
    return fresh + prediction.offset;
}

} // namespace rtype
