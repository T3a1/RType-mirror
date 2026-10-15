/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** packet_fuzz_test
*/

#include "common/protocol/Packet.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <map>
#include <random>
#include <vector>

using namespace rtype::protocol;

namespace {

using Bytes = std::vector<std::uint8_t>;

constexpr int Iterations = 200000;

Bytes encodeBytes(const Payload &payload)
{
    ByteWriter writer;
    EXPECT_TRUE(encodePacket(
        Packet{.header = {.sequence = 298, .ack = 17}, .payload = payload},
        writer));
    return {writer.data().begin(), writer.data().end()};
}

std::vector<Bytes> validPackets()
{
    SnapshotPart snapshot;
    snapshot.tick = 300;
    snapshot.players = {{.slot = 1, .score = 1500}};
    snapshot.entities.resize(3, {.networkId = 7, .health = 100, .x = 1.0F});
    SnapshotPart laterPart = snapshot;
    laterPart.part = 1;
    laterPart.partCount = 2;
    laterPart.players.clear();
    return {
        encodeBytes(Connect{}),
        encodeBytes(Accept{}),
        encodeBytes(Reject{}),
        encodeBytes(Disconnect{}),
        encodeBytes(Ack{}),
        encodeBytes(Input{.sequence = 1000, .actions = 0x11}),
        encodeBytes(Welcome{.slot = 2, .shipId = 58, .tick = 1200}),
        encodeBytes(snapshot),
        encodeBytes(laterPart),
        encodeBytes(GameEvent{.kind = GameEventKind::EntityDestroyed}),
        encodeBytes(GameEvent{.kind = GameEventKind::PlayerJoined, .slot = 1}),
    };
}

// One to four random edits: overwrite a byte, truncate, or append bytes.
// Half of the time the payload size is then fixed, so that the mutated
// packet gets past the header checks and reaches the payload decoders.
void mutate(Bytes &bytes, std::mt19937 &random)
{
    const auto edits = std::uniform_int_distribution(1, 4)(random);
    for (int i = 0; i < edits; ++i) {
        switch (std::uniform_int_distribution(0, 2)(random)) {
        case 0:
            if (!bytes.empty()) {
                bytes[random() % bytes.size()] =
                    static_cast<std::uint8_t>(random());
            }
            break;
        case 1:
            bytes.resize(random() % (bytes.size() + 1));
            break;
        default:
            bytes.resize(bytes.size() + (random() % 32),
                         static_cast<std::uint8_t>(random()));
            break;
        }
    }
    if (random() % 2 == 0 && bytes.size() >= HeaderSize) {
        const auto size = bytes.size() - HeaderSize;
        bytes[PayloadSizeOffset] = static_cast<std::uint8_t>(size);
        bytes[PayloadSizeOffset + 1] = static_cast<std::uint8_t>(size >> 8U);
    }
}

// A packet that decodes must encode back to the very same bytes: if it
// does not, a field was misread or ignored.
bool reencodesIdentically(const Packet &packet, const Bytes &bytes)
{
    ByteWriter writer;
    return encodePacket(packet, writer) &&
           std::ranges::equal(writer.data(), bytes);
}

struct Results {
    std::map<DecodeError, int> errors;
    int decoded = 0;
    int misread = 0; // decoded, but did not encode back to the same bytes
};

void decodeFromBothSides(const Bytes &bytes, Results &results)
{
    for (const Side sender : {Side::Client, Side::Server}) {
        const auto result = decodePacket(bytes, sender);
        if (const auto *error = std::get_if<DecodeError>(&result)) {
            ++results.errors[*error];
        } else if (reencodesIdentically(std::get<Packet>(result), bytes)) {
            ++results.decoded;
        } else {
            ++results.misread;
        }
    }
}

} // namespace

TEST(PacketFuzz, MutatedPacketsAreRejectedOrReadExactly)
{
    // Fixed seeds: a failure can be replayed.
    // NOLINTNEXTLINE(cert-msc32-c,cert-msc51-cpp,bugprone-random-generator-seed)
    std::mt19937 random(42);
    const auto seeds = validPackets();
    Results results;
    for (int i = 0; i < Iterations; ++i) {
        Bytes bytes = seeds[random() % seeds.size()];
        mutate(bytes, random);
        decodeFromBothSides(bytes, results);
    }
    EXPECT_EQ(results.misread, 0);
    // The mutations must reach every layer of the decoder.
    EXPECT_GT(results.decoded, 0);
    EXPECT_GT(results.errors[DecodeError::BadPayload], 0);
    EXPECT_GT(results.errors[DecodeError::BadPayloadSize], 0);
    EXPECT_GT(results.errors[DecodeError::WrongSender], 0);
}

TEST(PacketFuzz, RandomDatagramsAreRejected)
{
    // NOLINTNEXTLINE(cert-msc32-c,cert-msc51-cpp,bugprone-random-generator-seed)
    std::mt19937 random(7);
    for (int i = 0; i < Iterations; ++i) {
        Bytes bytes(random() % (MaxPacketSize + 100));
        std::ranges::generate(
            bytes, [&random] { return static_cast<std::uint8_t>(random()); });
        if (random() % 2 == 0 && bytes.size() >= 3) {
            bytes[0] = Magic[0];
            bytes[1] = Magic[1];
            bytes[2] = ProtocolVersion;
        }
        const auto result = decodePacket(bytes, Side::Client);
        if (const auto *packet = std::get_if<Packet>(&result)) {
            ASSERT_TRUE(reencodesIdentically(*packet, bytes))
                << "iteration " << i;
        }
    }
}
