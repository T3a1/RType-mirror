/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Messages
*/

#ifndef COMMON_PROTOCOL_MESSAGES_HPP
#define COMMON_PROTOCOL_MESSAGES_HPP

#include "common/components/EntityType.hpp"
#include "common/protocol/PacketType.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace rtype::protocol {

class ByteReader;
class ByteWriter;

inline constexpr std::uint8_t MaxPlayers = 4;

// Sizes on the wire, used to split snapshots.
inline constexpr std::size_t SnapshotHeaderSize = 8;
inline constexpr std::size_t PlayerCountSize = 1;
inline constexpr std::size_t PlayerRecordSize = 5;
inline constexpr std::size_t EntityRecordSize = 16;
inline constexpr std::size_t MaxSnapshotParts = 255;

struct Connect {
    static constexpr PacketType Type = PacketType::Connect;
    bool operator==(const Connect &) const = default;
};

struct Accept {
    static constexpr PacketType Type = PacketType::Accept;
    bool operator==(const Accept &) const = default;
};

enum class RejectReason : std::uint8_t {
    ServerFull,
    VersionMismatch,
};

struct Reject {
    static constexpr PacketType Type = PacketType::Reject;
    RejectReason reason = RejectReason::ServerFull;
    bool operator==(const Reject &) const = default;
};

enum class DisconnectReason : std::uint8_t {
    Left,
    Timeout,
    Kicked,
    ServerClosing,
};

// Best effort: a lost Disconnect ends in a timeout instead.
struct Disconnect {
    static constexpr PacketType Type = PacketType::Disconnect;
    DisconnectReason reason = DisconnectReason::Left;
    bool operator==(const Disconnect &) const = default;
};

struct Ack {
    static constexpr PacketType Type = PacketType::Ack;
    bool operator==(const Ack &) const = default;
};

struct Input {
    static constexpr PacketType Type = PacketType::Input;
    std::uint32_t sequence = 0;
    std::uint8_t actions = 0; // Action bits
    bool operator==(const Input &) const = default;
};

struct Welcome {
    static constexpr PacketType Type = PacketType::Welcome;
    std::uint8_t slot = 0; // 1 to MaxPlayers
    std::uint32_t shipId = 0;
    std::uint32_t tick = 0;
    bool operator==(const Welcome &) const = default;
};

struct PlayerInfo {
    std::uint8_t slot = 0; // 1 to MaxPlayers
    std::uint32_t score = 0;
    bool operator==(const PlayerInfo &) const = default;
};

namespace EntityFlag {
inline constexpr std::uint8_t Invincible = 1U << 0U;
inline constexpr std::uint8_t Charging = 1U << 1U;
inline constexpr std::uint8_t All = Invincible | Charging;
} // namespace EntityFlag

struct EntityState {
    std::uint32_t networkId = 0;
    EntityType type = EntityType::PlayerShip;
    std::uint8_t flags = 0;
    std::uint16_t health = 0;
    float x = 0.0; // center, in playfield units
    float y = 0.0;
    bool operator==(const EntityState &) const = default;
};

struct SnapshotPart {
    static constexpr PacketType Type = PacketType::Snapshot;
    std::uint32_t tick = 0;
    std::uint8_t part = 0;
    std::uint8_t partCount = 1;
    std::vector<PlayerInfo> players;
    std::vector<EntityState> entities;
    bool operator==(const SnapshotPart &) const = default;
};

enum class GameEventKind : std::uint8_t {
    EntityDestroyed, // networkId
    PlayerJoined,    // slot
    PlayerLeft,      // slot
    PlayerHit,       // slot
    GameOver,        // nothing
};

// Reliable. Only the field named next to the kind is sent.
struct GameEvent {
    static constexpr PacketType Type = PacketType::GameEvent;
    GameEventKind kind = GameEventKind::GameOver;
    std::uint32_t networkId = 0;
    std::uint8_t slot = 0;
    bool operator==(const GameEvent &) const = default;
};

void encode(const Connect &message, ByteWriter &out);
void encode(const Accept &message, ByteWriter &out);
void encode(const Reject &message, ByteWriter &out);
void encode(const Disconnect &message, ByteWriter &out);
void encode(const Ack &message, ByteWriter &out);
void encode(const Input &message, ByteWriter &out);
void encode(const Welcome &message, ByteWriter &out);
void encode(const SnapshotPart &message, ByteWriter &out);
void encode(const GameEvent &message, ByteWriter &out);

[[nodiscard]] bool decode(ByteReader &in, Connect &message);
[[nodiscard]] bool decode(ByteReader &in, Accept &message);
[[nodiscard]] bool decode(ByteReader &in, Reject &message);
[[nodiscard]] bool decode(ByteReader &in, Disconnect &message);
[[nodiscard]] bool decode(ByteReader &in, Ack &message);
[[nodiscard]] bool decode(ByteReader &in, Input &message);
[[nodiscard]] bool decode(ByteReader &in, Welcome &message);
[[nodiscard]] bool decode(ByteReader &in, SnapshotPart &message);
[[nodiscard]] bool decode(ByteReader &in, GameEvent &message);

// Cuts the state of one tick into parts that each fit in a datagram. Part
// 0 carries the players (at most MaxPlayers). Always returns at least one
// part; entities beyond MaxSnapshotParts parts are dropped.
[[nodiscard]] std::vector<SnapshotPart>
splitSnapshot(std::uint32_t tick, std::span<const PlayerInfo> players,
              std::span<const EntityState> entities);

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_MESSAGES_HPP
