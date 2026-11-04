/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NetworkSyncSystem
*/

#include "systems/NetworkSyncSystem.hpp"
#include "SpriteSheets.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/NetworkId.hpp"
#include "common/components/Player.hpp"
#include "common/components/SnapshotHistory.hpp"
#include "common/components/Transform.hpp"
#include "common/systems/PredictionSystem.hpp"
#include "components/ChargeState.hpp"
#include "components/ShipOwner.hpp"
#include "engine/Logger.hpp"

#include <algorithm>
#include <variant>

namespace client {

namespace {

using namespace rtype::protocol;

constexpr std::uint32_t DestroyedMemory = 300;

/**
 * @brief Returns the slot of the player owning a ship.
 * @param state The sync state, for the players block and the local ship.
 * @param networkId The NetworkId of the ship.
 * @return The owner's slot, 0 if no known player owns it.
 **/
std::uint8_t ownerOfShip(const SyncState &state, std::uint32_t networkId)
{
    if (networkId == state.localShip) {
        return state.localSlot;
    }
    for (const PlayerInfo &player : state.players) {
        if (player.shipId == networkId) {
            return player.slot;
        }
    }
    return 0;
}

/**
 * @brief Gives a ship the sprite and ShipOwner of its player, unless it
 * already has them.
 * @param registry The client's registry.
 * @param ship The ship entity.
 * @param slot The owner's slot, 0 if unknown (nothing is changed).
 * @param sheets The sprite sheets the ship is drawn with.
 **/
void setShipOwner(engine::Registry &registry, engine::Entity ship,
                  std::uint8_t slot, const SpriteSheets &sheets)
{
    const auto *owner = registry.tryGet<ShipOwner>(ship);
    if (slot == 0 || (owner != nullptr && owner->slot == slot)) {
        return;
    }
    attachSprite(registry, ship, sheets, rtype::EntityType::PlayerShip, slot);
    registry.emplace<ShipOwner>(ship, ShipOwner{.slot = slot});
}

/**
 * @brief Creates or updates the local copy of one snapshot entity, unless
 * it was destroyed recently. The position and health go to the entity's
 * history, or straight to the Transform and Health of the local ship. The
 * local player's missiles are drawn in the present, like the ship.
 * @param registry The client's registry.
 * @param entity The entity record from the snapshot.
 * @param tick The tick of the snapshot.
 * @param state The sync state.
 * @param sheets The sprite sheets the new entities are drawn with.
 **/
void applyEntity(engine::Registry &registry, const EntityState &entity,
                 std::uint32_t tick, SyncState &state,
                 const SpriteSheets &sheets)
{
    if (state.destroyed.contains(entity.networkId)) {
        return;
    }
    const rtype::SnapshotSample sample{
        .tick = tick,
        .position = {.x = entity.x, .y = entity.y},
        .health = entity.health,
    };
    const auto it = state.entities.find(entity.networkId);
    const ChargeState charge{.charging =
                                 (entity.flags & EntityFlag::Charging) != 0};
    if (it != state.entities.end()) {
        if (entity.type == rtype::EntityType::PlayerShip) {
            registry.emplace<ChargeState>(it->second, charge);
        }
        if (auto *history =
                registry.tryGet<rtype::SnapshotHistory>(it->second)) {
            rtype::pushSample(*history, sample);
            return;
        }
        if (entity.networkId == state.localShip && state.predict) {
            state.shipTick = tick;
            state.shipPosition = sample.position;
        } else {
            registry.get<rtype::Transform>(it->second).position =
                sample.position;
        }
        registry.get<rtype::Health>(it->second).current = sample.health;
        return;
    }
    const engine::Entity created = registry.create();
    registry.emplace<rtype::NetworkId>(created,
                                       rtype::NetworkId{entity.networkId});
    registry.emplace<rtype::EntityType>(created, entity.type);
    registry.emplace<rtype::Transform>(
        created, rtype::Transform{.position = sample.position});
    registry.emplace<rtype::Health>(
        created, rtype::Health{
                     .current = entity.health,
                     .max = entity.type == rtype::EntityType::Boss
                                ? rtype::BossMaxHealth
                                : rtype::PlayerMaxHealth,
                 });
    const bool local = entity.networkId == state.localShip;
    attachSprite(registry, created, sheets, entity.type, 0);
    if (entity.type == rtype::EntityType::PlayerShip) {
        registry.emplace<ChargeState>(created, charge);
        setShipOwner(registry, created, ownerOfShip(state, entity.networkId),
                     sheets);
    }
    if (local) {
        state.shipTick = tick;
        state.shipPosition = sample.position;
    }
    if (!local && state.interpolate) {
        const bool ownMissile =
            state.localSlot != 0 &&
            EntityFlag::ownerOf(entity.flags) == state.localSlot;
        rtype::SnapshotHistory history{
            .lead = ownMissile ? rtype::InterpolationDelay : 0.0,
        };
        rtype::pushSample(history, sample);
        registry.emplace<rtype::SnapshotHistory>(created, history);
    }
    state.entities.emplace(entity.networkId, created);
}

/**
 * @brief Applies one snapshot part if it is not older than the last one,
 * gives the known ships the color of their player, and forgets destroyed
 * ids older than DestroyedMemory ticks.
 * @param registry The client's registry.
 * @param snapshot The snapshot part.
 * @param state The sync state.
 * @param sheets The sprite sheets the new entities are drawn with.
 **/
void applySnapshot(engine::Registry &registry, const SnapshotPart &snapshot,
                   SyncState &state, const SpriteSheets &sheets)
{
    if (snapshot.tick < state.lastTick) {
        return;
    }
    state.lastTick = snapshot.tick;
    if (snapshot.part == 0) {
        state.players = snapshot.players;
        for (const PlayerInfo &player : snapshot.players) {
            if (player.slot == state.localSlot) {
                state.ackTick = snapshot.tick;
                state.ackInput = player.lastInput;
            }
            const auto ship = state.entities.find(player.shipId);
            if (ship != state.entities.end()) {
                setShipOwner(registry, ship->second, player.slot, sheets);
            }
        }
    }
    for (const EntityState &entity : snapshot.entities) {
        applyEntity(registry, entity, snapshot.tick, state, sheets);
    }
    std::erase_if(state.destroyed, [&state](const auto &entry) {
        return state.lastTick - entry.second > DestroyedMemory;
    });
}

/**
 * @brief Applies a game event: hands the entity of EntityDestroyed to the
 * dying list (or destroys it at once if it is not interpolated) and logs
 * the player events.
 * @param registry The client's registry.
 * @param event The event.
 * @param state The sync state.
 * @param sheets The sprite sheets explosions are drawn with.
 **/
void applyEvent(engine::Registry &registry, const GameEvent &event,
                SyncState &state, const SpriteSheets &sheets)
{
    switch (event.kind) {
    case GameEventKind::EntityDestroyed: {
        state.destroyed[event.networkId] = state.lastTick;
        const auto it = state.entities.find(event.networkId);
        if (it != state.entities.end()) {
            if (const auto *history =
                    registry.tryGet<rtype::SnapshotHistory>(it->second)) {
                state.dying.emplace_back(it->second,
                                         state.lastTick - history->lead);
            } else {
                spawnExplosion(registry, it->second, sheets);
                registry.destroy(it->second);
            }
            state.entities.erase(it);
        }
        break;
    }
    case GameEventKind::PlayerJoined:
        BB_INFO("Player " << static_cast<int>(event.slot) << " joined");
        break;
    case GameEventKind::PlayerLeft:
        BB_INFO("Player " << static_cast<int>(event.slot) << " left");
        break;
    case GameEventKind::PlayerHit:
        break;
    case GameEventKind::GameOver:
        BB_INFO("Game over");
        state.gameOver = true;
        break;
    case GameEventKind::GameStarted:
        BB_INFO("Game started");
        state.started = true;
        break;
    case GameEventKind::LevelStarted:
        state.level = event.level;
        state.levelAnnounced = true;
        state.levelBanner = LevelBannerTime;
        break;
    }
}

/**
 * @brief Records the local player's ship and slot, and gives the ship its
 * player's color and stops interpolating it if it already exists.
 * @param registry The client's registry.
 * @param welcome The Welcome payload.
 * @param state The sync state.
 * @param sheets The sprite sheets the ship is drawn with.
 **/
void applyWelcome(engine::Registry &registry, const Welcome &welcome,
                  SyncState &state, const SpriteSheets &sheets)
{
    BB_INFO("Playing as player " << static_cast<int>(welcome.slot));
    state.localShip = welcome.shipId;
    state.localSlot = welcome.slot;
    const auto it = state.entities.find(welcome.shipId);
    if (it != state.entities.end()) {
        setShipOwner(registry, it->second, welcome.slot, sheets);
        registry.remove<rtype::SnapshotHistory>(it->second);
    }
}

} // namespace

void networkSyncSystem(engine::Registry &registry,
                       const std::vector<Payload> &payloads, SyncState &state,
                       const SpriteSheets &sheets)
{
    for (const Payload &payload : payloads) {
        if (const auto *snapshot = std::get_if<SnapshotPart>(&payload)) {
            applySnapshot(registry, *snapshot, state, sheets);
        } else if (const auto *event = std::get_if<GameEvent>(&payload)) {
            applyEvent(registry, *event, state, sheets);
        } else if (const auto *welcome = std::get_if<Welcome>(&payload)) {
            applyWelcome(registry, *welcome, state, sheets);
        }
    }
    if (state.predict && state.shipTick == state.ackTick &&
        state.shipTick > state.acknowledgedTick) {
        rtype::acknowledgeInputs(state.prediction, state.ackInput,
                                 state.shipPosition);
        state.acknowledgedTick = state.shipTick;
    }
}

void smoothingSystem(engine::Registry &registry, SyncState &state,
                     engine::Duration dt, const SpriteSheets &sheets)
{
    rtype::advanceRenderClock(state.clock, state.lastTick, dt);
    rtype::interpolationSystem(registry, state.clock.tick);
    std::erase_if(state.dying, [&](const auto &dying) {
        if (state.clock.tick < dying.second) {
            return false;
        }
        spawnExplosion(registry, dying.first, sheets);
        registry.destroy(dying.first);
        return true;
    });
}

void predictionSystem(engine::Registry &registry, SyncState &state,
                      std::uint32_t sequence, std::uint8_t actions,
                      engine::Duration dt)
{
    if (!state.predict) {
        return;
    }
    rtype::recordInput(state.prediction, sequence, actions, dt);
    const auto position = rtype::predictShip(state.prediction, dt);
    const auto ship = state.entities.find(state.localShip);
    if (position && ship != state.entities.end()) {
        registry.get<rtype::Transform>(ship->second).position = *position;
    }
}

} // namespace client
