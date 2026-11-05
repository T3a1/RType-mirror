/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** room_phase_system_test
*/

#include "network/NetworkMessages.hpp"
#include "systems/RoomPhaseSystem.hpp"

#include <gtest/gtest.h>

using rtype::protocol::GameEvent;
using rtype::protocol::GameEventKind;
using rtype::protocol::RoomStatus;

namespace {

const server::InboundMessage StartMessage{
    .clientId = 1,
    .kind = server::InboundKind::Start,
    .settings = {
        .difficulty = rtype::protocol::Difficulty::Hard,
        .level = 2,
    }};
const server::InboundMessage ConnectedMessage{
    .clientId = 2, .kind = server::InboundKind::Connected};

} // namespace

TEST(RoomPhaseSystem, StartBeginsAWaitingGameForEveryone)
{
    server::Outbound outbound;
    RoomStatus status = RoomStatus::Waiting;

    const auto settings =
        server::roomPhaseSystem({StartMessage}, status, outbound);

    EXPECT_EQ(status, RoomStatus::Playing);
    EXPECT_EQ(settings, StartMessage.settings);
    const auto sent = outbound.drain();
    ASSERT_EQ(sent.size(), 1U);
    EXPECT_EQ(sent.front().to, server::Recipient::All);
    EXPECT_EQ(sent.front().payload, rtype::protocol::Payload{GameEvent{
                                        .kind = GameEventKind::GameStarted}});
}

TEST(RoomPhaseSystem, AnotherStartDoesNothing)
{
    server::Outbound outbound;
    RoomStatus status = RoomStatus::Playing;

    static_cast<void>(
        server::roomPhaseSystem({StartMessage}, status, outbound));

    EXPECT_EQ(outbound.size(), 0U);
}

TEST(RoomPhaseSystem, AFinishedGameDoesNotStartAgain)
{
    server::Outbound outbound;
    RoomStatus status = RoomStatus::Over;

    static_cast<void>(
        server::roomPhaseSystem({StartMessage}, status, outbound));

    EXPECT_EQ(status, RoomStatus::Over);
    EXPECT_EQ(outbound.size(), 0U);
}

TEST(RoomPhaseSystem, APlayerJoiningARunningGameIsToldAlone)
{
    server::Outbound outbound;
    RoomStatus status = RoomStatus::Playing;

    static_cast<void>(
        server::roomPhaseSystem({ConnectedMessage}, status, outbound));

    const auto sent = outbound.drain();
    ASSERT_EQ(sent.size(), 1U);
    EXPECT_EQ(sent.front().to, server::Recipient::One);
    EXPECT_EQ(sent.front().cliendId, 2U);
}

TEST(RoomPhaseSystem, APlayerJoiningAWaitingRoomIsNotTold)
{
    server::Outbound outbound;
    RoomStatus status = RoomStatus::Waiting;

    static_cast<void>(
        server::roomPhaseSystem({ConnectedMessage}, status, outbound));

    EXPECT_EQ(status, RoomStatus::Waiting);
    EXPECT_EQ(outbound.size(), 0U);
}
