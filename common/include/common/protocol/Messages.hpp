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

/**
 * @struct Connect
 * @brief A client asks to join; repeated until Accept or Reject. Empty.
 * @param Type The packet type that carries this payload.
 **/
struct Connect {
    static constexpr PacketType Type = PacketType::Connect;
    bool operator==(const Connect &) const = default;
};

/**
 * @struct Accept
 * @brief The server accepts the client and opens its session. Empty.
 * @param Type The packet type that carries this payload.
 **/
struct Accept {
    static constexpr PacketType Type = PacketType::Accept;
    bool operator==(const Accept &) const = default;
};

/**
 * @enum RejectReason
 * @brief Why the server refuses a client.
 * @param ServerFull MaxPlayers players are already connected.
 * @param VersionMismatch The client speaks another protocol version.
 **/
enum class RejectReason : std::uint8_t {
    ServerFull,
    VersionMismatch,
};

/**
 * @struct Reject
 * @brief The server refuses a client; no session is created.
 * @param Type The packet type that carries this payload.
 * @param reason Why the client is refused.
 **/
struct Reject {
    static constexpr PacketType Type = PacketType::Reject;
    RejectReason reason = RejectReason::ServerFull;
    bool operator==(const Reject &) const = default;
};

/**
 * @enum DisconnectReason
 * @brief Why a connection ended.
 * @param Left The player quit.
 * @param Timeout Nothing was received from the peer for PeerTimeout.
 * @param Kicked The peer broke the protocol.
 * @param ServerClosing The server is shutting down.
 **/
enum class DisconnectReason : std::uint8_t {
    Left,
    Timeout,
    Kicked,
    ServerClosing,
};

/**
 * @struct Disconnect
 * @brief Either side ends the connection. Best effort: a lost Disconnect
 * ends in a timeout instead.
 * @param Type The packet type that carries this payload.
 * @param reason Why the connection ends.
 **/
struct Disconnect {
    static constexpr PacketType Type = PacketType::Disconnect;
    DisconnectReason reason = DisconnectReason::Left;
    bool operator==(const Disconnect &) const = default;
};

/**
 * @struct Ack
 * @brief Carries only the header's ack, when a side has reliable packets to
 * acknowledge and nothing else to send. Empty.
 * @param Type The packet type that carries this payload.
 **/
struct Ack {
    static constexpr PacketType Type = PacketType::Ack;
    bool operator==(const Ack &) const = default;
};

/**
 * @struct Input
 * @brief The actions a player holds; sent every frame by the client.
 * @param Type The packet type that carries this payload.
 * @param sequence Increases with every input, so the server can drop older
 * ones that arrive late.
 * @param actions The held actions, as Action bits.
 **/
struct Input {
    static constexpr PacketType Type = PacketType::Input;
    std::uint32_t sequence = 0;
    std::uint8_t actions = 0;
    bool operator==(const Input &) const = default;
};

/**
 * @struct Welcome
 * @brief Tells a new player who it is in the game (reliable).
 * @param Type The packet type that carries this payload.
 * @param slot The player's slot, from 1 to MaxPlayers.
 * @param shipId The NetworkId of the player's ship.
 * @param tick The server tick when the player joined.
 **/
struct Welcome {
    static constexpr PacketType Type = PacketType::Welcome;
    std::uint8_t slot = 0;
    std::uint32_t shipId = 0;
    std::uint32_t tick = 0;
    bool operator==(const Welcome &) const = default;
};

/**
 * @struct PlayerInfo
 * @brief One player in the players block of a snapshot.
 * @param slot The player's slot, from 1 to MaxPlayers.
 * @param score The player's score, in points.
 **/
struct PlayerInfo {
    std::uint8_t slot = 0;
    std::uint32_t score = 0;
    bool operator==(const PlayerInfo &) const = default;
};

namespace EntityFlag {
inline constexpr std::uint8_t Invincible = 1U << 0U;
inline constexpr std::uint8_t Charging = 1U << 1U;
inline constexpr std::uint8_t All = Invincible | Charging;
} // namespace EntityFlag

/**
 * @struct EntityState
 * @brief One entity in a snapshot (EntityRecordSize bytes on the wire).
 * @param networkId The id shared by the server and the clients.
 * @param type What the entity is, to pick its sprite.
 * @param flags EntityFlag bits.
 * @param health The entity's health points, 0 if it has none.
 * @param x The horizontal position of its center, in playfield units.
 * @param y The vertical position of its center, in playfield units.
 **/
struct EntityState {
    std::uint32_t networkId = 0;
    EntityType type = EntityType::PlayerShip;
    std::uint8_t flags = 0;
    std::uint16_t health = 0;
    float x = 0.0;
    float y = 0.0;
    bool operator==(const EntityState &) const = default;
};

/**
 * @struct SnapshotPart
 * @brief One self-contained part of the world state at a tick (unreliable).
 * @param Type The packet type that carries this payload.
 * @param tick The server tick the state belongs to.
 * @param part The index of this part, from 0 to partCount - 1.
 * @param partCount The number of parts of this tick, at least 1.
 * @param players The players block; only part 0 carries it.
 * @param entities The entities of this part.
 **/
struct SnapshotPart {
    static constexpr PacketType Type = PacketType::Snapshot;
    std::uint32_t tick = 0;
    std::uint8_t part = 0;
    std::uint8_t partCount = 1;
    std::vector<PlayerInfo> players;
    std::vector<EntityState> entities;
    bool operator==(const SnapshotPart &) const = default;
};

/**
 * @enum GameEventKind
 * @brief What a GameEvent reports, which also selects the field it sends.
 * @param EntityDestroyed An entity is gone; sends networkId.
 * @param PlayerJoined A player joined; sends slot.
 * @param PlayerLeft A player left; sends slot.
 * @param PlayerHit A player was hit; sends slot.
 * @param GameOver The game is over; sends nothing.
 **/
enum class GameEventKind : std::uint8_t {
    EntityDestroyed,
    PlayerJoined,
    PlayerLeft,
    PlayerHit,
    GameOver,
};

/**
 * @struct GameEvent
 * @brief A one-off game fact (reliable). Only the field its kind names is
 * sent.
 * @param Type The packet type that carries this payload.
 * @param kind What happened.
 * @param networkId The destroyed entity, for EntityDestroyed.
 * @param slot The player concerned, for the player events.
 **/
struct GameEvent {
    static constexpr PacketType Type = PacketType::GameEvent;
    GameEventKind kind = GameEventKind::GameOver;
    std::uint32_t networkId = 0;
    std::uint8_t slot = 0;
    bool operator==(const GameEvent &) const = default;
};

/**
 * @brief Writes the payload of a Connect.
 * @param message The payload to write; writes nothing (empty payload).
 * @param out The writer to append to.
 **/
void encode(const Connect &message, ByteWriter &out);
/**
 * @brief Writes the payload of a Accept.
 * @param message The payload to write; writes nothing (empty payload).
 * @param out The writer to append to.
 **/
void encode(const Accept &message, ByteWriter &out);
/**
 * @brief Writes the payload of a Reject.
 * @param message The payload to write; writes its fields.
 * @param out The writer to append to.
 **/
void encode(const Reject &message, ByteWriter &out);
/**
 * @brief Writes the payload of a Disconnect.
 * @param message The payload to write; writes its fields.
 * @param out The writer to append to.
 **/
void encode(const Disconnect &message, ByteWriter &out);
/**
 * @brief Writes the payload of a Ack.
 * @param message The payload to write; writes nothing (empty payload).
 * @param out The writer to append to.
 **/
void encode(const Ack &message, ByteWriter &out);
/**
 * @brief Writes the payload of a Input.
 * @param message The payload to write; writes its fields.
 * @param out The writer to append to.
 **/
void encode(const Input &message, ByteWriter &out);
/**
 * @brief Writes the payload of a Welcome.
 * @param message The payload to write; writes its fields.
 * @param out The writer to append to.
 **/
void encode(const Welcome &message, ByteWriter &out);
/**
 * @brief Writes the payload of a SnapshotPart.
 * @param message The payload to write; writes its fields.
 * @param out The writer to append to.
 **/
void encode(const SnapshotPart &message, ByteWriter &out);
/**
 * @brief Writes the payload of a GameEvent.
 * @param message The payload to write; writes its fields.
 * @param out The writer to append to.
 **/
void encode(const GameEvent &message, ByteWriter &out);

/**
 * @brief Reads and validates the payload of a Connect.
 * @param in The reader positioned at the start of the payload.
 * @param message Filled with the decoded fields.
 * @return `true` if the fields are valid; the caller still checks
 * in.ok() and that no byte is left.
 **/
[[nodiscard]] bool decode(ByteReader &in, Connect &message);
/**
 * @brief Reads and validates the payload of a Accept.
 * @param in The reader positioned at the start of the payload.
 * @param message Filled with the decoded fields.
 * @return `true` if the fields are valid; the caller still checks
 * in.ok() and that no byte is left.
 **/
[[nodiscard]] bool decode(ByteReader &in, Accept &message);
/**
 * @brief Reads and validates the payload of a Reject. Fails on an unknown
 * reason.
 * @param in The reader positioned at the start of the payload.
 * @param message Filled with the decoded fields.
 * @return `true` if the fields are valid; the caller still checks
 * in.ok() and that no byte is left.
 **/
[[nodiscard]] bool decode(ByteReader &in, Reject &message);
/**
 * @brief Reads and validates the payload of a Disconnect. Fails on an unknown
 * reason.
 * @param in The reader positioned at the start of the payload.
 * @param message Filled with the decoded fields.
 * @return `true` if the fields are valid; the caller still checks
 * in.ok() and that no byte is left.
 **/
[[nodiscard]] bool decode(ByteReader &in, Disconnect &message);
/**
 * @brief Reads and validates the payload of a Ack.
 * @param in The reader positioned at the start of the payload.
 * @param message Filled with the decoded fields.
 * @return `true` if the fields are valid; the caller still checks
 * in.ok() and that no byte is left.
 **/
[[nodiscard]] bool decode(ByteReader &in, Ack &message);
/**
 * @brief Reads and validates the payload of a Input. Fails on unknown action
 * bits.
 * @param in The reader positioned at the start of the payload.
 * @param message Filled with the decoded fields.
 * @return `true` if the fields are valid; the caller still checks
 * in.ok() and that no byte is left.
 **/
[[nodiscard]] bool decode(ByteReader &in, Input &message);
/**
 * @brief Reads and validates the payload of a Welcome. Fails on a slot outside
 * 1 to MaxPlayers.
 * @param in The reader positioned at the start of the payload.
 * @param message Filled with the decoded fields.
 * @return `true` if the fields are valid; the caller still checks
 * in.ok() and that no byte is left.
 **/
[[nodiscard]] bool decode(ByteReader &in, Welcome &message);
/**
 * @brief Reads and validates the payload of a SnapshotPart. Fails on a part
 * index out of range, more than MaxPlayers players, players outside part
 * 0, an entity count larger than the remaining bytes (checked before
 * allocating), an unknown entity type or flag, or a position that is not
 * finite.
 * @param in The reader positioned at the start of the payload.
 * @param message Filled with the decoded fields.
 * @return `true` if the fields are valid; the caller still checks
 * in.ok() and that no byte is left.
 **/
[[nodiscard]] bool decode(ByteReader &in, SnapshotPart &message);
/**
 * @brief Reads and validates the payload of a GameEvent. Fails on an unknown
 * kind or an invalid slot.
 * @param in The reader positioned at the start of the payload.
 * @param message Filled with the decoded fields.
 * @return `true` if the fields are valid; the caller still checks
 * in.ok() and that no byte is left.
 **/
[[nodiscard]] bool decode(ByteReader &in, GameEvent &message);

/**
 * @brief Cuts the state of one tick into parts that each fit in a datagram.
 * @param tick The server tick of the state.
 * @param players The players; part 0 carries at most MaxPlayers of them.
 * @param entities The entities, spread over the parts in order.
 * @return At least one part. Entities that do not fit in MaxSnapshotParts
 * parts are dropped.
 **/
[[nodiscard]] std::vector<SnapshotPart>
splitSnapshot(std::uint32_t tick, std::span<const PlayerInfo> players,
              std::span<const EntityState> entities);

} // namespace rtype::protocol

#endif // COMMON_PROTOCOL_MESSAGES_HPP
