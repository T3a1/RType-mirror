/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** SnapshotSystem
*/

#include "systems/SnapshotSystem.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/NetworkId.hpp"
#include "common/components/Player.hpp"
#include "common/components/Projectile.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Weapon.hpp"
#include "common/protocol/Messages.hpp"
#include "components/PowerUp.hpp"
#include "components/ShotStyles.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace server {

namespace {

/** @brief How long a player holds the fire key before the clients see it
 * charge, in seconds. */
constexpr engine::Duration ChargingFlagDelay{0.3F};

/**
 * @brief Returns the health of an entity as sent in a snapshot.
 * @param registry The registry holding the entity.
 * @param entity The entity.
 * @return Its health points clamped to 0..65535, or 0 if it has no Health.
 **/
std::uint16_t healthOf(engine::Registry &registry, engine::Entity entity)
{
    const auto *health = registry.tryGet<rtype::Health>(entity);
    if (health == nullptr) {
        return 0;
    }
    return static_cast<std::uint16_t>(std::clamp(
        health->current, 0, int{std::numeric_limits<std::uint16_t>::max()}));
}

/**
 * @brief Returns the flags of an entity as sent in a snapshot.
 * @param registry The registry holding the entity.
 * @param entity The entity.
 * @return The Owner bits of a projectile fired by a player still alive,
 * Charging for a player's ship whose fire key is held for a while, the
 * Boost bits of a power-up, 0 otherwise.
 **/
std::uint8_t flagsOf(engine::Registry &registry, engine::Entity entity)
{
    if (const auto *powerUp = registry.tryGet<PowerUp>(entity)) {
        return rtype::protocol::EntityFlag::boost(powerUp->kind);
    }
    const auto *projectile = registry.tryGet<rtype::Projectile>(entity);
    if (projectile == nullptr) {
        const auto *weapon = registry.tryGet<rtype::Weapon>(entity);
        const bool charging = weapon != nullptr &&
                              registry.has<rtype::Player>(entity) &&
                              weapon->charge >= ChargingFlagDelay;
        return charging ? rtype::protocol::EntityFlag::Charging : 0;
    }
    const auto *player = registry.tryGet<rtype::Player>(projectile->owner);
    return player != nullptr ? rtype::protocol::EntityFlag::owner(player->slot)
                             : 0;
}

/**
 * @brief Builds the snapshot record of every networked entity.
 * @param registry The registry to read.
 * @return One record per entity with a NetworkId, an EntityType and a
 * Transform.
 **/
std::vector<rtype::protocol::EntityState>
entityStates(engine::Registry &registry)
{
    std::vector<rtype::protocol::EntityState> states;
    registry.each<rtype::NetworkId, rtype::EntityType, rtype::Transform>(
        [&](engine::Entity entity, const rtype::NetworkId &networkId,
            const rtype::EntityType &type, const rtype::Transform &transform) {
            states.push_back({
                .networkId = networkId.value,
                .type = type,
                .flags = flagsOf(registry, entity),
                .health = healthOf(registry, entity),
                .x = transform.position.x,
                .y = transform.position.y,
            });
        });
    return states;
}

/**
 * @brief Returns the boost timers of a ship as sent in a snapshot.
 * @param registry The registry holding the ship.
 * @param ship The ship.
 * @return The seconds left of each boost, rounded up, 0 for none.
 **/
std::array<std::uint8_t, rtype::PowerUpKindCount>
boostsOf(engine::Registry &registry, engine::Entity ship)
{
    std::array<std::uint8_t, rtype::PowerUpKindCount> seconds{};
    const auto *boosts = registry.tryGet<Boosts>(ship);
    if (boosts == nullptr) {
        return seconds;
    }
    for (std::size_t i = 0; i < seconds.size(); ++i) {
        seconds.at(i) = static_cast<std::uint8_t>(
            std::clamp(std::ceil(boosts->left.at(i).count()), 0.0F,
                       float{std::numeric_limits<std::uint8_t>::max()}));
    }
    return seconds;
}

/**
 * @brief Builds the players block of a snapshot.
 * @param registry The registry holding the ships.
 * @param players The connected players.
 * @return One record per player; the ship id is 0 if its ship is gone.
 **/
std::vector<rtype::protocol::PlayerInfo> playerInfos(engine::Registry &registry,
                                                     const Players &players)
{
    std::vector<rtype::protocol::PlayerInfo> infos;
    for (const auto &[clientId, player] : players) {
        const auto *networkId = registry.tryGet<rtype::NetworkId>(player.ship);
        infos.push_back({
            .slot = player.slot,
            .score = player.score,
            .lastInput = player.lastSequence,
            .shipId = networkId != nullptr ? networkId->value : 0,
            .boosts = boostsOf(registry, player.ship),
        });
    }
    return infos;
}

} // namespace

void snapshotSystem(engine::Registry &registry, const Players &players,
                    Outbound &outbound, std::uint32_t tick)
{
    const auto entities = entityStates(registry);
    const auto infos = playerInfos(registry, players);
    for (rtype::protocol::SnapshotPart &part :
         rtype::protocol::splitSnapshot(tick, infos, entities)) {
        outbound.push(OutboundMessage{
            .to = Recipient::All,
            .payload = std::move(part),
        });
    }
}

} // namespace server
