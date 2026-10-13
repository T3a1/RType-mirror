/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** packet_test
*/

#include "common/components/InputState.hpp"
#include "common/protocol/Messages.hpp"
#include "common/protocol/Packet.hpp"
#include "common/protocol/PacketHeader.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <limits>
#include <vector>

using namespace rtype::protocol;
using rtype::EntityType;

namespace {

std::vector<std::uint8_t> encodeBytes(const Packet &packet)
{
    ByteWriter writer;
    EXPECT_TRUE(encodePacket(packet, writer));
    return {writer.data().begin(), writer.data().end()};
}

PacketType typeOf(const Payload &payload)
{
    return std::visit(
        [](const auto &message) {
            return std::remove_cvref_t<decltype(message)>::Type;
        },
        payload);
}

Side senderOf(const Payload &payload)
{
    return canSend(Side::Client, typeOf(payload)) ? Side::Client : Side::Server;
}

DecodeError decodeError(const std::vector<std::uint8_t> &bytes, Side sender)
{
    const auto result = decodePacket(bytes, sender);
    EXPECT_TRUE(std::holds_alternative<DecodeError>(result));
    if (const auto *error = std::get_if<DecodeError>(&result)) {
        return *error;
    }
    return DecodeError::TooShort;
}

Packet packetOf(Payload payload)
{
    return Packet{
        .header =
            {
                .sequence = 298,
                .ack = 17,
            },
        .payload = std::move(payload),
    };
}

// Header values encodePacket fills in, for comparing a decoded packet.
Packet withComputedHeader(Packet packet, const std::vector<std::uint8_t> &bytes)
{
    packet.header.type = typeOf(packet.payload);
    packet.header.flags =
        isReliable(packet.header.type) ? PacketFlag::Reliable : 0;
    packet.header.payloadSize =
        static_cast<std::uint16_t>(bytes.size() - HeaderSize);
    return packet;
}

SnapshotPart sampleSnapshot()
{
    SnapshotPart snapshot;
    snapshot.tick = 300;
    snapshot.players = {
        {.slot = 1, .score = 1500},
        {.slot = 2, .score = 0},
    };
    snapshot.entities = {
        {
            .networkId = 7,
            .type = EntityType::PlayerShip,
            .flags = EntityFlag::Invincible,
            .health = 100,
            .x = 200.0F,
            .y = 540.0F,
        },
        {
            .networkId = 42,
            .type = EntityType::EnemyWave,
            .health = 20,
            .x = 1950.5F,
            .y = 120.25F,
        },
    };
    return snapshot;
}

bool decodesFrom(const Payload &payload, Side sender)
{
    return std::holds_alternative<Packet>(
        decodePacket(encodeBytes(packetOf(payload)), sender));
}

void expectBadPayload(const Payload &payload)
{
    EXPECT_EQ(decodeError(encodeBytes(packetOf(payload)), senderOf(payload)),
              DecodeError::BadPayload);
}

// Encodes a valid packet, then overwrites one byte with an invalid value.
void expectBadByte(const Payload &payload, std::size_t offset,
                   std::uint8_t value)
{
    auto bytes = encodeBytes(packetOf(payload));
    ASSERT_LT(offset, bytes.size());
    bytes[offset] = value;
    EXPECT_EQ(decodeError(bytes, senderOf(payload)), DecodeError::BadPayload);
}

void expectSendablePart(const SnapshotPart &part, std::uint8_t partCount)
{
    EXPECT_EQ(part.partCount, partCount);
    EXPECT_EQ(part.players.empty(), part.part != 0);
    ByteWriter writer;
    EXPECT_TRUE(encodePacket(packetOf(part), writer));
}

} // namespace

TEST(Packet, MatchesTheDocumentedLayout)
{
    const auto bytes = encodeBytes(packetOf(Input{
        .sequence = 1000,
        .actions = rtype::Action::Up | rtype::Action::Fire,
    }));
    const std::vector<std::uint8_t> expected{
        0x52, 0x54,             // magic "RT"
        0x01,                   // version
        0x10,                   // type Input
        0x00,                   // flags
        0x00,                   // reserved
        0x2A, 0x01,             // sequence 298
        0x11, 0x00,             // ack 17
        0x05, 0x00,             // payload size 5
        0xE8, 0x03, 0x00, 0x00, // input sequence 1000
        0x11,                   // Up | Fire
    };
    EXPECT_EQ(bytes, expected);
}

TEST(Packet, RoundTripsEveryPayload)
{
    SnapshotPart laterPart = sampleSnapshot();
    laterPart.part = 1;
    laterPart.partCount = 2;
    laterPart.players.clear();
    const std::vector<Payload> payloads{
        Connect{},
        Accept{},
        Reject{.reason = RejectReason::VersionMismatch},
        Disconnect{.reason = DisconnectReason::ServerClosing},
        Ack{},
        Input{.sequence = 0xFFFFFFFF, .actions = rtype::Action::All},
        Welcome{.slot = 4, .shipId = 99, .tick = 123456},
        sampleSnapshot(),
        laterPart,
        GameEvent{.kind = GameEventKind::EntityDestroyed, .networkId = 42},
        GameEvent{.kind = GameEventKind::PlayerLeft, .slot = 3},
        GameEvent{.kind = GameEventKind::GameOver},
    };
    for (const Payload &payload : payloads) {
        SCOPED_TRACE(payload.index());
        const Packet packet = packetOf(payload);
        const auto bytes = encodeBytes(packet);
        const auto result = decodePacket(bytes, senderOf(payload));
        ASSERT_TRUE(std::holds_alternative<Packet>(result));
        EXPECT_EQ(std::get<Packet>(result), withComputedHeader(packet, bytes));
    }
}

TEST(Packet, RejectsEveryTruncation)
{
    const auto bytes = encodeBytes(packetOf(sampleSnapshot()));
    for (std::size_t size = 0; size < bytes.size(); ++size) {
        SCOPED_TRACE(size);
        const std::vector<std::uint8_t> truncated(
            bytes.begin(), bytes.begin() + static_cast<std::ptrdiff_t>(size));
        EXPECT_TRUE(std::holds_alternative<DecodeError>(
            decodePacket(truncated, Side::Server)));
    }
}

TEST(Packet, RejectsAPayloadSizeThatDoesNotMatch)
{
    auto bytes = encodeBytes(packetOf(Ack{}));
    bytes.push_back(0);
    EXPECT_EQ(decodeError(bytes, Side::Client), DecodeError::BadPayloadSize);
}

TEST(Packet, RejectsTrailingBytesInThePayload)
{
    auto bytes = encodeBytes(packetOf(Ack{}));
    bytes.push_back(0);
    bytes[PayloadSizeOffset] = 1;
    EXPECT_EQ(decodeError(bytes, Side::Client), DecodeError::BadPayload);
}

TEST(Packet, RejectsBadHeaders)
{
    const auto valid = encodeBytes(packetOf(Ack{}));
    struct BadByte {
        std::size_t offset;
        std::uint8_t value;
        DecodeError error;
    };
    const std::array cases{
        BadByte{.offset = 0, .value = 'X', .error = DecodeError::BadMagic},
        BadByte{.offset = 1, .value = 'X', .error = DecodeError::BadMagic},
        BadByte{.offset = 2, .value = 2, .error = DecodeError::BadVersion},
        BadByte{.offset = 3, .value = 0x7F, .error = DecodeError::UnknownType},
        BadByte{.offset = 4, .value = 0x80, .error = DecodeError::BadFlags},
        BadByte{.offset = 5, .value = 1, .error = DecodeError::BadReserved},
    };
    for (const auto &badCase : cases) {
        SCOPED_TRACE(badCase.offset);
        auto bytes = valid;
        bytes[badCase.offset] = badCase.value;
        EXPECT_EQ(decodeError(bytes, Side::Client), badCase.error);
    }
}

TEST(Packet, MarksReliableTypesReliable)
{
    const auto bytes =
        encodeBytes(packetOf(Welcome{.slot = 2, .shipId = 58, .tick = 1200}));
    const std::vector<std::uint8_t> expected{
        0x52, 0x54, 0x01, 0x21, // magic, version, type Welcome
        0x01, 0x00,             // flags Reliable, reserved
        0x2A, 0x01, 0x11, 0x00, // sequence 298, ack 17
        0x09, 0x00,             // payload size 9
        0x02,                   // slot 2
        0x3A, 0x00, 0x00, 0x00, // ship 58
        0xB0, 0x04, 0x00, 0x00, // tick 1200
    };
    EXPECT_EQ(bytes, expected);
}

TEST(Packet, RejectsAReliableFlagThatDoesNotMatchTheType)
{
    auto unreliableEvent = encodeBytes(packetOf(GameEvent{}));
    unreliableEvent[4] = 0;
    EXPECT_EQ(decodeError(unreliableEvent, Side::Server),
              DecodeError::BadFlags);
    auto reliableInput = encodeBytes(packetOf(Input{}));
    reliableInput[4] = PacketFlag::Reliable;
    EXPECT_EQ(decodeError(reliableInput, Side::Client), DecodeError::BadFlags);
}

TEST(Packet, RejectsPacketsFromTheWrongSide)
{
    EXPECT_EQ(decodeError(encodeBytes(packetOf(Input{})), Side::Server),
              DecodeError::WrongSender);
    EXPECT_EQ(decodeError(encodeBytes(packetOf(Connect{})), Side::Server),
              DecodeError::WrongSender);
    EXPECT_EQ(
        decodeError(encodeBytes(packetOf(sampleSnapshot())), Side::Client),
        DecodeError::WrongSender);
    EXPECT_EQ(
        decodeError(encodeBytes(packetOf(Welcome{.slot = 1})), Side::Client),
        DecodeError::WrongSender);
}

TEST(Packet, AcceptsAckAndDisconnectFromBothSides)
{
    EXPECT_TRUE(decodesFrom(Ack{}, Side::Client));
    EXPECT_TRUE(decodesFrom(Ack{}, Side::Server));
    EXPECT_TRUE(decodesFrom(Disconnect{}, Side::Client));
    EXPECT_TRUE(decodesFrom(Disconnect{}, Side::Server));
}

TEST(Packet, RejectsDatagramsOfTheWrongSize)
{
    EXPECT_EQ(decodeError({}, Side::Client), DecodeError::TooShort);
    EXPECT_EQ(
        decodeError(std::vector<std::uint8_t>(HeaderSize - 1, 0), Side::Client),
        DecodeError::TooShort);
    EXPECT_EQ(decodeError(std::vector<std::uint8_t>(MaxPacketSize + 1, 0),
                          Side::Client),
              DecodeError::TooLarge);
}

TEST(Packet, RejectsAnEntityCountLargerThanThePacket)
{
    ByteWriter writer;
    encodeHeader({.type = PacketType::Snapshot, .payloadSize = 9}, writer);
    writer.u32(300);    // tick
    writer.u8(0);       // part
    writer.u8(1);       // partCount
    writer.u16(0xFFFF); // entityCount, but no records follow
    writer.u8(0);       // playerCount
    const std::vector<std::uint8_t> bytes(writer.data().begin(),
                                          writer.data().end());
    EXPECT_EQ(decodeError(bytes, Side::Server), DecodeError::BadPayload);
}

TEST(Packet, RejectsOutOfRangeFields)
{
    expectBadPayload(Input{.actions = 0x80});
    expectBadPayload(Welcome{.slot = 0});
    expectBadPayload(Welcome{.slot = MaxPlayers + 1});
    expectBadPayload(GameEvent{.kind = GameEventKind::PlayerJoined, .slot = 0});
}

TEST(Packet, RejectsUnknownEnumValues)
{
    const Reject reject;
    const Disconnect disconnect;
    const GameEvent event;
    expectBadByte(reject, HeaderSize, 9);
    expectBadByte(disconnect, HeaderSize, 9);
    expectBadByte(event, HeaderSize, 9);
    // Snapshot header (8) + 2 players (1 + 2 * 5) + networkId (4).
    expectBadByte(sampleSnapshot(), HeaderSize + 23, 200);
}

TEST(Packet, RejectsInvalidSnapshots)
{
    SnapshotPart snapshot = sampleSnapshot();
    snapshot.entities[1].x = std::numeric_limits<float>::quiet_NaN();
    expectBadPayload(snapshot);

    snapshot = sampleSnapshot();
    snapshot.entities[0].y = std::numeric_limits<float>::infinity();
    expectBadPayload(snapshot);

    snapshot = sampleSnapshot();
    snapshot.entities[0].flags = 0x80;
    expectBadPayload(snapshot);

    snapshot = sampleSnapshot();
    snapshot.part = 2;
    snapshot.partCount = 2;
    expectBadPayload(snapshot);

    snapshot = sampleSnapshot();
    snapshot.partCount = 0;
    expectBadPayload(snapshot);

    snapshot = sampleSnapshot();
    snapshot.players.resize(MaxPlayers + 1, {.slot = 1});
    expectBadPayload(snapshot);

    snapshot = sampleSnapshot();
    snapshot.players[0].slot = 0;
    expectBadPayload(snapshot);
}

TEST(Packet, RefusesToEncodeMoreThanMaxPacketSize)
{
    SnapshotPart huge;
    huge.entities.resize(100);
    ByteWriter writer;
    EXPECT_FALSE(encodePacket(packetOf(huge), writer));
}

TEST(SplitSnapshot, AlwaysReturnsAPartWithThePlayers)
{
    const std::vector<PlayerInfo> players{{.slot = 1, .score = 10}};
    const auto parts = splitSnapshot(5, players, {});
    ASSERT_EQ(parts.size(), 1U);
    EXPECT_EQ(parts[0].tick, 5U);
    EXPECT_EQ(parts[0].partCount, 1);
    EXPECT_EQ(parts[0].players, players);
    EXPECT_TRUE(parts[0].entities.empty());
}

TEST(SplitSnapshot, EveryPartFitsInADatagram)
{
    const std::vector<PlayerInfo> players{
        {.slot = 1},
        {.slot = 2},
        {.slot = 3},
        {.slot = 4},
    };
    std::vector<EntityState> entities(200);
    for (std::size_t i = 0; i < entities.size(); ++i) {
        entities[i].networkId = static_cast<std::uint32_t>(i);
    }
    const auto parts = splitSnapshot(300, players, entities);
    ASSERT_EQ(parts.size(), 3U);
    EXPECT_EQ(parts[0].entities.size(), 72U);
    EXPECT_EQ(parts[1].entities.size(), 73U);
    EXPECT_EQ(parts[2].entities.size(), 55U);
    std::vector<EntityState> received;
    for (const SnapshotPart &part : parts) {
        expectSendablePart(part, 3);
        received.insert(received.end(), part.entities.begin(),
                        part.entities.end());
    }
    EXPECT_EQ(received, entities);
}

TEST(Sequence, NewerAcrossTheWrap)
{
    EXPECT_TRUE(isNewer(1, 0));
    EXPECT_FALSE(isNewer(0, 1));
    EXPECT_FALSE(isNewer(5, 5));
    EXPECT_TRUE(isNewer(0, 65535));
    EXPECT_TRUE(isNewer(10, 65530));
    EXPECT_FALSE(isNewer(65530, 10));
}
