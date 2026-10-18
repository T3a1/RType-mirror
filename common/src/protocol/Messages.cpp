/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Messages
*/

#include "common/protocol/Messages.hpp"
#include "common/components/InputState.hpp"
#include "common/protocol/ByteReader.hpp"
#include "common/protocol/ByteWriter.hpp"
#include "common/protocol/PacketHeader.hpp"

#include <algorithm>
#include <cmath>

namespace rtype::protocol {

namespace {

/**
 * @brief Checks a player slot read from the network.
 * @param slot The slot to check.
 * @return `true` if it is between 1 and MaxPlayers.
 **/
bool isValidSlot(std::uint8_t slot) { return slot >= 1 && slot <= MaxPlayers; }

/**
 * @brief Reads a one-byte enum whose values run from 0 to `last`.
 * @param in The reader to read from.
 * @param last The highest valid value of the enum.
 * @param out Set to the value read if it is valid.
 * @return `true` if the byte is a valid value.
 **/
template <typename Enum> bool readEnum(ByteReader &in, Enum last, Enum &out)
{
    const std::uint8_t value = in.u8();
    if (value > static_cast<std::uint8_t>(last)) {
        return false;
    }
    out = static_cast<Enum>(value);
    return true;
}

} // namespace

void encode(const Connect & /*message*/, ByteWriter & /*out*/) {}

void encode(const Accept & /*message*/, ByteWriter & /*out*/) {}

void encode(const Reject &message, ByteWriter &out)
{
    out.u8(static_cast<std::uint8_t>(message.reason));
}

void encode(const Disconnect &message, ByteWriter &out)
{
    out.u8(static_cast<std::uint8_t>(message.reason));
}

void encode(const Ack & /*message*/, ByteWriter & /*out*/) {}

void encode(const Input &message, ByteWriter &out)
{
    out.u32(message.sequence);
    out.u8(message.actions);
}

void encode(const Welcome &message, ByteWriter &out)
{
    out.u8(message.slot);
    out.u32(message.shipId);
    out.u32(message.tick);
}

void encode(const SnapshotPart &message, ByteWriter &out)
{
    out.u32(message.tick);
    out.u8(message.part);
    out.u8(message.partCount);
    out.u16(static_cast<std::uint16_t>(message.entities.size()));
    if (message.part == 0) {
        out.u8(static_cast<std::uint8_t>(message.players.size()));
        for (const PlayerInfo &player : message.players) {
            out.u8(player.slot);
            out.u32(player.score);
        }
    }
    for (const EntityState &entity : message.entities) {
        out.u32(entity.networkId);
        out.u8(static_cast<std::uint8_t>(entity.type));
        out.u8(entity.flags);
        out.u16(entity.health);
        out.f32(entity.x);
        out.f32(entity.y);
    }
}

void encode(const GameEvent &message, ByteWriter &out)
{
    out.u8(static_cast<std::uint8_t>(message.kind));
    switch (message.kind) {
    case GameEventKind::EntityDestroyed:
        out.u32(message.networkId);
        break;
    case GameEventKind::PlayerJoined:
    case GameEventKind::PlayerLeft:
    case GameEventKind::PlayerHit:
        out.u8(message.slot);
        break;
    case GameEventKind::GameOver:
        break;
    }
}

bool decode(ByteReader & /*in*/, Connect & /*message*/) { return true; }

bool decode(ByteReader & /*in*/, Accept & /*message*/) { return true; }

bool decode(ByteReader &in, Reject &message)
{
    return readEnum(in, RejectReason::VersionMismatch, message.reason);
}

bool decode(ByteReader &in, Disconnect &message)
{
    return readEnum(in, DisconnectReason::ServerClosing, message.reason);
}

bool decode(ByteReader & /*in*/, Ack & /*message*/) { return true; }

bool decode(ByteReader &in, Input &message)
{
    message.sequence = in.u32();
    message.actions = in.u8();
    return (message.actions & Action::All) == message.actions;
}

bool decode(ByteReader &in, Welcome &message)
{
    message.slot = in.u8();
    message.shipId = in.u32();
    message.tick = in.u32();
    return isValidSlot(message.slot);
}

bool decode(ByteReader &in, SnapshotPart &message)
{
    message.tick = in.u32();
    message.part = in.u8();
    message.partCount = in.u8();
    const std::uint16_t entityCount = in.u16();
    if (message.partCount == 0 || message.part >= message.partCount) {
        return false;
    }
    if (message.part == 0) {
        const std::uint8_t playerCount = in.u8();
        if (playerCount > MaxPlayers) {
            return false;
        }
        for (std::uint8_t i = 0; i < playerCount; ++i) {
            PlayerInfo player;
            player.slot = in.u8();
            player.score = in.u32();
            if (!isValidSlot(player.slot)) {
                return false;
            }
            message.players.push_back(player);
        }
    }
    // Checked before reserving: a small packet claiming 65535 entities
    // must not make us allocate for them.
    if (entityCount > in.remaining() / EntityRecordSize) {
        return false;
    }
    message.entities.reserve(entityCount);
    for (std::uint16_t i = 0; i < entityCount; ++i) {
        EntityState entity;
        entity.networkId = in.u32();
        if (!readEnum(in, EntityType::EnemyMissile, entity.type)) {
            return false;
        }
        entity.flags = in.u8();
        entity.health = in.u16();
        entity.x = in.f32();
        entity.y = in.f32();
        if ((entity.flags & EntityFlag::All) != entity.flags ||
            !std::isfinite(entity.x) || !std::isfinite(entity.y)) {
            return false;
        }
        message.entities.push_back(entity);
    }
    return true;
}

bool decode(ByteReader &in, GameEvent &message)
{
    if (!readEnum(in, GameEventKind::GameOver, message.kind)) {
        return false;
    }
    switch (message.kind) {
    case GameEventKind::EntityDestroyed:
        message.networkId = in.u32();
        return true;
    case GameEventKind::PlayerJoined:
    case GameEventKind::PlayerLeft:
    case GameEventKind::PlayerHit:
        message.slot = in.u8();
        return isValidSlot(message.slot);
    case GameEventKind::GameOver:
        return true;
    }
    return false;
}

std::vector<SnapshotPart> splitSnapshot(std::uint32_t tick,
                                        std::span<const PlayerInfo> players,
                                        std::span<const EntityState> entities)
{
    const auto partPlayers =
        players.first(std::min(players.size(), std::size_t{MaxPlayers}));
    std::vector<SnapshotPart> parts;
    std::size_t next = 0;
    while (parts.empty() ||
           (next < entities.size() && parts.size() < MaxSnapshotParts)) {
        SnapshotPart part;
        part.tick = tick;
        part.part = static_cast<std::uint8_t>(parts.size());
        std::size_t room = MaxPayloadSize - SnapshotHeaderSize;
        if (parts.empty()) {
            part.players.assign(partPlayers.begin(), partPlayers.end());
            room -= PlayerCountSize + (partPlayers.size() * PlayerRecordSize);
        }
        const auto chunk = entities.subspan(
            next, std::min(room / EntityRecordSize, entities.size() - next));
        part.entities.assign(chunk.begin(), chunk.end());
        next += chunk.size();
        parts.push_back(std::move(part));
    }
    for (SnapshotPart &part : parts) {
        part.partCount = static_cast<std::uint8_t>(parts.size());
    }
    return parts;
}

} // namespace rtype::protocol
