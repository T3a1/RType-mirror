/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** prediction_system_test
*/

#include "common/Playfield.hpp"
#include "common/components/InputState.hpp"
#include "common/components/Prediction.hpp"
#include "common/systems/PlayerControlSystem.hpp"
#include "common/systems/PredictionSystem.hpp"

#include <gtest/gtest.h>

namespace {

constexpr engine::Duration Frame{0.01F};

} // namespace

TEST(PredictionSystem, MovesTheShipLikeTheServer)
{
    const engine::Vec2f moved =
        rtype::moveShip({.x = 500.0F, .y = 500.0F}, rtype::Action::Up, 0.1F);

    EXPECT_FLOAT_EQ(moved.x, 500.0F);
    EXPECT_FLOAT_EQ(moved.y, 500.0F - (rtype::playerSpeed * 0.1F));
}

TEST(PredictionSystem, KeepsTheShipInThePlayfield)
{
    const engine::Vec2f moved =
        rtype::moveShip({.x = 40.0F, .y = 500.0F}, rtype::Action::Left, 1.0F);

    EXPECT_FLOAT_EQ(moved.x, rtype::PlayerShipSize.x / 2.0F);
}

TEST(PredictionSystem, IgnoresInputsThatWereNotSent)
{
    rtype::Prediction prediction;
    rtype::recordInput(prediction, 0, rtype::Action::Up, Frame);

    EXPECT_TRUE(prediction.pending.empty());
}

TEST(PredictionSystem, KeepsAtMostMaxPendingInputs)
{
    rtype::Prediction prediction;
    for (std::uint32_t sequence = 1;
         sequence <= rtype::Prediction::MaxPending + 5; ++sequence) {
        rtype::recordInput(prediction, sequence, 0, Frame);
    }

    EXPECT_EQ(prediction.pending.size(), rtype::Prediction::MaxPending);
    EXPECT_EQ(prediction.pending.front().sequence, 6U);
}

TEST(PredictionSystem, AcknowledgingDropsTheAppliedInputs)
{
    rtype::Prediction prediction;
    for (std::uint32_t sequence = 1; sequence <= 5; ++sequence) {
        rtype::recordInput(prediction, sequence, 0, Frame);
    }

    rtype::acknowledgeInputs(prediction, 3, {.x = 1.0F, .y = 2.0F});

    ASSERT_EQ(prediction.pending.size(), 2U);
    EXPECT_EQ(prediction.pending.front().sequence, 4U);
    EXPECT_EQ(prediction.acknowledged, 3U);
}

TEST(PredictionSystem, IgnoresAnOlderAcknowledgement)
{
    rtype::Prediction prediction;
    rtype::acknowledgeInputs(prediction, 10, {.x = 1.0F, .y = 2.0F});

    rtype::acknowledgeInputs(prediction, 8, {.x = 9.0F, .y = 9.0F});

    EXPECT_EQ(prediction.acknowledged, 10U);
    EXPECT_FLOAT_EQ(prediction.authoritative.value_or(engine::Vec2f{}).x, 1.0F);
}

TEST(PredictionSystem, PredictsNothingBeforeTheFirstSnapshot)
{
    rtype::Prediction prediction;
    rtype::recordInput(prediction, 1, rtype::Action::Up, Frame);

    EXPECT_FALSE(rtype::predictShip(prediction, Frame).has_value());
}

TEST(PredictionSystem, ReplaysThePendingInputsOnTheServerPosition)
{
    rtype::Prediction prediction;
    rtype::acknowledgeInputs(prediction, 0, {.x = 500.0F, .y = 500.0F});
    for (std::uint32_t sequence = 1; sequence <= 10; ++sequence) {
        rtype::recordInput(prediction, sequence, rtype::Action::Right, Frame);
    }

    const auto position = rtype::predictShip(prediction, Frame);

    ASSERT_TRUE(position.has_value());
    EXPECT_NEAR(position->x, 500.0F + (rtype::playerSpeed * 0.1F), 1e-3);
    EXPECT_FLOAT_EQ(position->y, 500.0F);
}

TEST(PredictionSystem, NoCorrectionWhenTheServerAgrees)
{
    rtype::Prediction prediction;
    rtype::acknowledgeInputs(prediction, 0, {.x = 500.0F, .y = 500.0F});
    engine::Vec2f server{.x = 500.0F, .y = 500.0F};
    for (std::uint32_t sequence = 1; sequence <= 20; ++sequence) {
        rtype::recordInput(prediction, sequence, rtype::Action::Down, Frame);
        static_cast<void>(rtype::predictShip(prediction, Frame));
        EXPECT_NEAR(prediction.lastCorrection, 0.0F, 1e-3);
        if (sequence % 5 == 0) {
            server =
                rtype::moveShip(server, rtype::Action::Down, 5 * Frame.count());
            rtype::acknowledgeInputs(prediction, sequence, server);
        }
    }
}

TEST(PredictionSystem, SmoothsACorrection)
{
    rtype::Prediction prediction;
    rtype::acknowledgeInputs(prediction, 0, {.x = 500.0F, .y = 500.0F});
    rtype::recordInput(prediction, 1, 0, Frame);
    const auto before = rtype::predictShip(prediction, Frame);

    rtype::acknowledgeInputs(prediction, 1, {.x = 520.0F, .y = 500.0F});
    const auto after = rtype::predictShip(prediction, Frame);

    ASSERT_TRUE(before && after);
    EXPECT_FLOAT_EQ(prediction.lastCorrection, 20.0F);
    EXPECT_GT(after->x, before->x);
    EXPECT_LT(after->x - before->x, 2.5F);
    for (int frame = 0; frame < 100; ++frame) {
        static_cast<void>(rtype::predictShip(prediction, Frame));
    }
    EXPECT_NEAR(
        rtype::predictShip(prediction, Frame).value_or(engine::Vec2f{}).x,
        520.0F, 0.01F);
}

TEST(PredictionSystem, AppliesAHugeCorrectionAtOnce)
{
    rtype::Prediction prediction;
    rtype::acknowledgeInputs(prediction, 0, {.x = 500.0F, .y = 500.0F});
    static_cast<void>(rtype::predictShip(prediction, Frame));

    rtype::acknowledgeInputs(prediction, 0, {.x = 1500.0F, .y = 500.0F});
    const auto position = rtype::predictShip(prediction, Frame);

    ASSERT_TRUE(position.has_value());
    EXPECT_FLOAT_EQ(position->x, 1500.0F);
}
