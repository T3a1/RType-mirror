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

/** @brief How many players a room holds, at most. */
inline constexpr std::uint8_t MaxPlayers = 4;
/** @brief How many rooms a server runs at the same time, at most. */
inline constexpr std::uint8_t MaxRooms = 16;
/** @brief Asks for the first room with a free slot, or a new one if none. */
inline constexpr std::uint32_t AnyRoom = 0;
/** @brief Asks for a new room. */
inline constexpr std::uint32_t NewRoom = 0xFFFFFFFF;

/** @brief The size of the fixed part of a snapshot on the wire, in bytes. */
inline constexpr std::size_t SnapshotHeaderSize = 8;
/** @brief The size of the number of players on the wire, in bytes. */
inline constexpr std::size_t PlayerCountSize = 1;
/** @brief The size of one player on the wire, in bytes. */
inline constexpr std::size_t PlayerRecordSize = 13;
/** @brief The size of one entity on the wire, in bytes. */
inline constexpr std::size_t EntityRecordSize = 16;
/** @brief How many parts one snapshot can be split into, at most. */
inline constexpr std::size_t MaxSnapshotParts = 255;

/**
 * @struct Connect
 * @brief A client asks to enter the lobby; repeated until Accept or Reject.
 * Empty.
 * @param Type The packet type that carries this payload.
 **/
struct Connect {
    static constexpr PacketType Type = PacketType::Connect;
    bool operator==(const Connect &) const = default;
};

/**
 * @struct Accept
 * @brief The server accepts the client, which is now in the lobby. Empty.
 * @param Type The packet type that carries this payload.
 **/
struct Accept {
    static constexpr PacketType Type = PacketType::Accept;
    bool operator==(const Accept &) const = default;
};

/**
 * @enum RejectReason
 * @brief Why the server refuses a client.
 * @param ServerFull The server has as many clients as all its rooms can
 * hold.
 * @param VersionMismatch The client speaks another protocol version.
 * @param Banned The address of the client is banned.
 * @param NotWhitelisted The server only accepts listed addresses.
 **/
enum class RejectReason : std::uint8_t {
    ServerFull,
    VersionMismatch,
    Banned,
    NotWhitelisted,
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
 * @param Kicked The peer broke the protocol, or an administrator removed it.
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
 * @param lastInput The sequence of the player's last Input applied by the
 * server at this tick, 0 if none, for client-side prediction.
 * @param shipId The NetworkId of the player's ship, 0 if it has none, so
 * that clients can tell which ship belongs to which player.
 **/
struct PlayerInfo {
    std::uint8_t slot = 0;
    std::uint32_t score = 0;
    std::uint32_t lastInput = 0;
    std::uint32_t shipId = 0;
    bool operator==(const PlayerInfo &) const = default;
};

/** @brief The bits of the flags of an entity in a snapshot. */
namespace EntityFlag {
/** @brief The entity cannot be hurt. */
inline constexpr std::uint8_t Invincible = 1U << 0U;
/** @brief The player's ship is charging a shot. */
inline constexpr std::uint8_t Charging = 1U << 1U;
/** @brief The position of the Owner bits. */
inline constexpr unsigned OwnerShift = 2U;
/** @brief The bits holding the slot of the player who owns the entity. */
inline constexpr std::uint8_t Owner = 0x7U << OwnerShift;
/** @brief Every known bit. */
inline constexpr std::uint8_t All =
    unsigned{Invincible} | unsigned{Charging} | unsigned{Owner};

/**
 * @brief Returns the flag bits naming the player who owns an entity.
 * @param slot The owner's slot, from 1 to MaxPlayers, or 0 for none.
 * @return The Owner bits.
 **/
constexpr std::uint8_t owner(std::uint8_t slot)
{
    return static_cast<std::uint8_t>((unsigned{slot} << OwnerShift) &
                                     unsigned{Owner});
}

/**
 * @brief Returns the slot of the player who owns an entity.
 * @param flags The entity's flags.
 * @return The owner's slot, or 0 if no player owns it.
 **/
constexpr std::uint8_t ownerOf(std::uint8_t flags)
{
    return static_cast<std::uint8_t>((unsigned{flags} & unsigned{Owner}) >>
                                     OwnerShift);
}
} // namespace EntityFlag

/**
 * @struct EntityState
 * @brief One entity in a snapshot (EntityRecordSize bytes on the wire).
 * @param networkId The id shared by the server and the clients.
 * @param type What the entity is, to pick its sprite.
 * @param flags EntityFlag bits: Invincible, Charging, and the Owner slot
 * of a player's missile.
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
 * @param GameStarted The game of the room started; sends nothing.
 * @param LevelStarted A level is about to start; sends level.
 **/
enum class GameEventKind : std::uint8_t {
    EntityDestroyed,
    PlayerJoined,
    PlayerLeft,
    PlayerHit,
    GameOver,
    GameStarted,
    LevelStarted,
};

/**
 * @struct GameEvent
 * @brief A one-off game fact (reliable). Only the field its kind names is
 * sent.
 * @param Type The packet type that carries this payload.
 * @param kind What happened.
 * @param networkId The destroyed entity, for EntityDestroyed.
 * @param slot The player concerned, for the player events.
 * @param level The level that starts, from 1, or 0 for the endless mode,
 * for LevelStarted.
 **/
struct GameEvent {
    static constexpr PacketType Type = PacketType::GameEvent;
    GameEventKind kind = GameEventKind::GameOver;
    std::uint32_t networkId = 0;
    std::uint8_t slot = 0;
    std::uint8_t level = 0;
    bool operator==(const GameEvent &) const = default;
};

/**
 * @enum RoomStatus
 * @brief Where the game of a room stands.
 * @param Waiting The players wait for one of them to start it.
 * @param Playing The game runs; new players may still join.
 * @param Over The game is over; nobody can join.
 **/
enum class RoomStatus : std::uint8_t {
    Waiting,
    Playing,
    Over,
};

/**
 * @struct RoomInfo
 * @brief One room in a RoomList.
 * @param room The room's number, at least 1.
 * @param players How many players are in it, at most MaxPlayers.
 * @param status Where its game stands.
 **/
struct RoomInfo {
    std::uint32_t room = 0;
    std::uint8_t players = 0;
    RoomStatus status = RoomStatus::Waiting;
    bool operator==(const RoomInfo &) const = default;
};

/**
 * @struct RoomList
 * @brief The rooms of the server, sent regularly to the clients in the lobby.
 * @param Type The packet type that carries this payload.
 * @param rooms The rooms, at most MaxRooms.
 **/
struct RoomList {
    static constexpr PacketType Type = PacketType::RoomList;
    std::vector<RoomInfo> rooms;
    bool operator==(const RoomList &) const = default;
};

/**
 * @struct JoinRoom
 * @brief A client in the lobby asks to enter a room (reliable).
 * @param Type The packet type that carries this payload.
 * @param room The room, created if it does not exist, or AnyRoom or NewRoom.
 **/
struct JoinRoom {
    static constexpr PacketType Type = PacketType::JoinRoom;
    std::uint32_t room = AnyRoom;
    bool operator==(const JoinRoom &) const = default;
};

/**
 * @struct LeaveRoom
 * @brief A client leaves its room and goes back to the lobby (reliable).
 * Empty.
 * @param Type The packet type that carries this payload.
 **/
struct LeaveRoom {
    static constexpr PacketType Type = PacketType::LeaveRoom;
    bool operator==(const LeaveRoom &) const = default;
};

/**
 * @enum Difficulty
 * @brief How hard a game is: enemy health, fire rate and, in endless mode,
 * how often they come.
 **/
enum class Difficulty : std::uint8_t { Easy, Normal, Hard };

/**
 * @enum BossChoice
 * @brief Which boss ends the levels that have one.
 * @param LevelBoss The boss written in each level file.
 * @param Warship The warship, in every level that has a boss.
 * @param Hive The hive, in every level that has a boss.
 **/
enum class BossChoice : std::uint8_t { LevelBoss, Warship, Hive };

/** @brief StartGame::level to skip the levels and play the endless mode. */
inline constexpr std::uint8_t EndlessLevel = 0;

/**
 * @struct StartGame
 * @brief A player starts the game of its room, with its settings
 * (reliable).
 * @param Type The packet type that carries this payload.
 * @param difficulty How hard the game is.
 * @param level The level it starts at, from 1, or EndlessLevel; a level
 * past the last one starts the last one.
 * @param boss Which boss ends the levels.
 **/
struct StartGame {
    static constexpr PacketType Type = PacketType::StartGame;
    Difficulty difficulty = Difficulty::Normal;
    std::uint8_t level = 1;
    BossChoice boss = BossChoice::LevelBoss;
    bool operator==(const StartGame &) const = default;
};

/**
 * @struct RoomJoined
 * @brief The server put the client in a room (reliable).
 * @param Type The packet type that carries this payload.
 * @param room The room's number, at least 1.
 * @param levels How many levels the server has, for the choice of the
 * starting level; 0 if it only has the endless mode.
 **/
struct RoomJoined {
    static constexpr PacketType Type = PacketType::RoomJoined;
    std::uint32_t room = 1;
    std::uint8_t levels = 0;
    bool operator==(const RoomJoined &) const = default;
};

/**
 * @struct RoomRefused
 * @brief The room is full or its game is over, or the server cannot open
 * another room; the client stays in the lobby (reliable). Empty.
 * @param Type The packet type that carries this payload.
 **/
struct RoomRefused {
    static constexpr PacketType Type = PacketType::RoomRefused;
    bool operator==(const RoomRefused &) const = default;
};

/**
 * @struct RoomClosed
 * @brief An administrator closed the room of the client, which is back in the
 * lobby (reliable). Empty.
 * @param Type The packet type that carries this payload.
 **/
struct RoomClosed {
    static constexpr PacketType Type = PacketType::RoomClosed;
    bool operator==(const RoomClosed &) const = default;
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
 * @brief Writes the payload of a RoomList.
 * @param message The message.
 * @param out Where to write.
 **/
void encode(const RoomList &message, ByteWriter &out);
/**
 * @brief Writes the payload of a JoinRoom.
 * @param message The message.
 * @param out Where to write.
 **/
void encode(const JoinRoom &message, ByteWriter &out);
/**
 * @brief Writes the payload of a LeaveRoom.
 * @param message The message.
 * @param out Where to write.
 **/
void encode(const LeaveRoom &message, ByteWriter &out);
/**
 * @brief Writes the payload of a StartGame.
 * @param message The message.
 * @param out Where to write.
 **/
void encode(const StartGame &message, ByteWriter &out);
/**
 * @brief Writes the payload of a RoomJoined.
 * @param message The message.
 * @param out Where to write.
 **/
void encode(const RoomJoined &message, ByteWriter &out);
/**
 * @brief Writes the payload of a RoomRefused.
 * @param message The message.
 * @param out Where to write.
 **/
void encode(const RoomRefused &message, ByteWriter &out);
/**
 * @brief Writes the payload of a RoomClosed.
 * @param message The message.
 * @param out Where to write.
 **/
void encode(const RoomClosed &message, ByteWriter &out);
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
 * @brief Reads and validates the payload of a RoomList.
 * @param in Where to read.
 * @param message The message to fill.
 * @return Whether the values are valid.
 **/
[[nodiscard]] bool decode(ByteReader &in, RoomList &message);
/**
 * @brief Reads the payload of a JoinRoom.
 * @param in Where to read.
 * @param message The message to fill.
 * @return Whether the values are valid.
 **/
[[nodiscard]] bool decode(ByteReader &in, JoinRoom &message);
/**
 * @brief Reads the payload of a LeaveRoom, which is empty.
 * @param in Where to read.
 * @param message The message to fill.
 * @return Always true.
 **/
[[nodiscard]] bool decode(ByteReader &in, LeaveRoom &message);
/**
 * @brief Reads the payload of a StartGame, which is empty.
 * @param in Where to read.
 * @param message The message to fill.
 * @return Always true.
 **/
[[nodiscard]] bool decode(ByteReader &in, StartGame &message);
/**
 * @brief Reads and validates the payload of a RoomJoined.
 * @param in Where to read.
 * @param message The message to fill.
 * @return Whether the room is not 0.
 **/
[[nodiscard]] bool decode(ByteReader &in, RoomJoined &message);
/**
 * @brief Reads the payload of a RoomRefused, which is empty.
 * @param in Where to read.
 * @param message The message to fill.
 * @return Always true.
 **/
[[nodiscard]] bool decode(ByteReader &in, RoomRefused &message);
/**
 * @brief Reads the payload of a RoomClosed, which is empty.
 * @param in Where to read.
 * @param message The message to fill.
 * @return Always true.
 **/
[[nodiscard]] bool decode(ByteReader &in, RoomClosed &message);
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
