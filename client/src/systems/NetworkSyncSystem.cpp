/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NetworkSyncSystem
*/

#include "systems/NetworkSyncSystem.hpp"
#include "SpriteSheets.hpp"
#include "common/Playfield.hpp"
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
#include <optional>
#include <variant>

namespace client {

namespace {

using namespace rtype::protocol;

/** @brief How long a destroyed id is remembered, in server ticks. */
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
 * @brief Returns the sound of an entity that just appeared.
 * @param type What the entity is.
 * @return The sound, if it has one.
 **/
std::optional<audio::SoundEvent> spawnSound(rtype::EntityType type)
{
    switch (type) {
    case rtype::EntityType::PlayerMissile:
        return audio::SoundEvent::PlayerShot;
    case rtype::EntityType::ChargedShot:
        return audio::SoundEvent::ChargedShot;
    case rtype::EntityType::EnemyMissile:
        return audio::SoundEvent::EnemyShot;
    case rtype::EntityType::Boss:
        return audio::SoundEvent::BossAlert;
    case rtype::EntityType::PowerUp:
        return audio::SoundEvent::PowerUpSpawn;
    default:
        return std::nullopt;
    }
}

/**
 * @brief Plays the hit sound if an enemy lost health. A player's ship does not
 * count: it is destroyed by its first hit.
 * @param entity The entity record from the snapshot.
 * @param previous The health the entity had.
 * @param sounds The sounds of the game.
 **/
void playHitSound(const EntityState &entity, int previous,
                  audio::SoundBoard &sounds)
{
    if (entity.type != rtype::EntityType::PlayerShip &&
        entity.health < previous) {
        sounds.play(audio::SoundEvent::EnemyHit);
    }
}

/**
 * @brief Creates the explosion of a destroyed entity and plays its sound, which
 * depends on what the entity was.
 * @param registry The client's registry.
 * @param entity The destroyed entity.
 * @param state The sync state, for the local ship.
 * @param sheets The sprite sheets the explosion is drawn with.
 * @param sounds The sounds of the game.
 **/
void explode(engine::Registry &registry, engine::Entity entity,
             const SyncState &state, const SpriteSheets &sheets,
             audio::SoundBoard &sounds)
{
    if (!spawnExplosion(registry, entity, sheets)) {
        return;
    }
    const auto *networkId = registry.tryGet<rtype::NetworkId>(entity);
    if (networkId != nullptr && networkId->value == state.localShip) {
        sounds.play(audio::SoundEvent::PlayerDeath);
        return;
    }
    switch (registry.get<rtype::EntityType>(entity)) {
    case rtype::EntityType::Boss:
        sounds.play(audio::SoundEvent::ExplosionBoss);
        break;
    case rtype::EntityType::PlayerShip:
    case rtype::EntityType::EnemyMedium:
    case rtype::EntityType::EnemyLarge:
        sounds.play(audio::SoundEvent::ExplosionLarge);
        break;
    default:
        sounds.play(audio::SoundEvent::Explosion);
        break;
    }
}

/**
 * @brief Starts the charge sound when the local ship starts charging, and
 * stops it when the ship stops. Call it before updating the ChargeState.
 * @param registry The client's registry.
 * @param ship The local ship.
 * @param charge The new charge state of the ship.
 * @param sounds The sounds of the game.
 **/
void updateChargeSound(engine::Registry &registry, engine::Entity ship,
                       const ChargeState &charge, audio::SoundBoard &sounds)
{
    const auto *previous = registry.tryGet<ChargeState>(ship);
    const bool wasCharging = previous != nullptr && previous->charging;
    if (charge.charging && !wasCharging) {
        sounds.play(audio::SoundEvent::Charge);
    } else if (!charge.charging && wasCharging) {
        sounds.stop(audio::SoundEvent::Charge);
    }
}

/**
 * @brief Tells whether a destroyed entity is a power-up taken by a player: a
 * power-up only disappears inside the playfield when it is taken.
 * @param registry The client's registry.
 * @param entity The destroyed entity.
 * @return True if a player took it.
 **/
bool isTakenPowerUp(engine::Registry &registry, engine::Entity entity)
{
    const auto *type = registry.tryGet<rtype::EntityType>(entity);
    const auto *transform = registry.tryGet<rtype::Transform>(entity);
    return type != nullptr && transform != nullptr &&
           *type == rtype::EntityType::PowerUp &&
           transform->position.x >= 0.0F &&
           transform->position.x <= rtype::PlayfieldWidth &&
           transform->position.y >= 0.0F &&
           transform->position.y <= rtype::PlayfieldHeight;
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
 * @param sounds The sounds played for new entities, hits and the charge.
 **/
void applyEntity(engine::Registry &registry, const EntityState &entity,
                 std::uint32_t tick, SyncState &state,
                 const SpriteSheets &sheets, audio::SoundBoard &sounds)
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
            if (entity.networkId == state.localShip) {
                updateChargeSound(registry, it->second, charge, sounds);
            }
            registry.emplace<ChargeState>(it->second, charge);
        }
        if (auto *history =
                registry.tryGet<rtype::SnapshotHistory>(it->second)) {
            if (history->count > 0) {
                playHitSound(entity,
                             history->samples.at(history->count - 1).health,
                             sounds);
            }
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
        auto &health = registry.get<rtype::Health>(it->second);
        playHitSound(entity, health.current, sounds);
        health.current = sample.health;
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
                     // A boss arrives with its full health, which depends
                     // on the difficulty.
                     .max = rtype::isBossEntity(entity.type)
                                ? std::max<int>(entity.health, 1)
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
    if (const auto sound = spawnSound(entity.type)) {
        sounds.play(*sound);
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
 * @param sounds The sounds played for new entities, hits and the charge.
 **/
void applySnapshot(engine::Registry &registry, const SnapshotPart &snapshot,
                   SyncState &state, const SpriteSheets &sheets,
                   audio::SoundBoard &sounds)
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
        applyEntity(registry, entity, snapshot.tick, state, sheets, sounds);
    }
    std::erase_if(state.destroyed, [&state](const auto &entry) {
        return state.lastTick - entry.second > DestroyedMemory;
    });
}

/**
 * @brief Applies a game event: hands the entity of EntityDestroyed to the
 * dying list (or destroys it at once if it is not interpolated), and logs
 * the player events and adds a notice for them.
 * @param registry The client's registry.
 * @param event The event.
 * @param state The sync state.
 * @param sheets The sprite sheets explosions are drawn with.
 * @param sounds The sounds played for the event.
 **/
void applyEvent(engine::Registry &registry, const GameEvent &event,
                SyncState &state, const SpriteSheets &sheets,
                audio::SoundBoard &sounds)
{
    switch (event.kind) {
    case GameEventKind::EntityDestroyed: {
        state.destroyed[event.networkId] = state.lastTick;
        const auto it = state.entities.find(event.networkId);
        if (it != state.entities.end()) {
            if (isTakenPowerUp(registry, it->second)) {
                sounds.play(audio::SoundEvent::PowerUp);
            }
            if (const auto *history =
                    registry.tryGet<rtype::SnapshotHistory>(it->second)) {
                state.dying.emplace_back(it->second,
                                         state.lastTick - history->lead);
            } else {
                explode(registry, it->second, state, sheets, sounds);
                registry.destroy(it->second);
            }
            state.entities.erase(it);
        }
        break;
    }
    case GameEventKind::PlayerJoined:
        BB_INFO("Player " << static_cast<int>(event.slot) << " joined");
        addPlayerNotice(state.notices, event.slot, true);
        sounds.play(audio::SoundEvent::PlayerJoined);
        break;
    case GameEventKind::PlayerLeft:
        BB_INFO("Player " << static_cast<int>(event.slot) << " left");
        addPlayerNotice(state.notices, event.slot, false);
        sounds.play(audio::SoundEvent::PlayerLeft);
        break;
    case GameEventKind::PlayerHit:
        break;
    case GameEventKind::GameOver:
        BB_INFO("Game over");
        state.gameOver = true;
        sounds.play(audio::SoundEvent::GameOver);
        break;
    case GameEventKind::GameStarted:
        BB_INFO("Game started");
        state.started = true;
        sounds.play(audio::SoundEvent::GameStart);
        break;
    case GameEventKind::LevelStarted:
        state.level = event.level;
        state.levelAnnounced = true;
        state.levelBanner = LevelBannerTime;
        sounds.play(audio::SoundEvent::LevelStart);
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
                       const SpriteSheets &sheets, audio::SoundBoard &sounds)
{
    for (const Payload &payload : payloads) {
        if (const auto *snapshot = std::get_if<SnapshotPart>(&payload)) {
            applySnapshot(registry, *snapshot, state, sheets, sounds);
        } else if (const auto *event = std::get_if<GameEvent>(&payload)) {
            applyEvent(registry, *event, state, sheets, sounds);
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
                     engine::Duration dt, const SpriteSheets &sheets,
                     audio::SoundBoard &sounds)
{
    rtype::advanceRenderClock(state.clock, state.lastTick, dt);
    rtype::interpolationSystem(registry, state.clock.tick);
    std::erase_if(state.dying, [&](const auto &dying) {
        if (state.clock.tick < dying.second) {
            return false;
        }
        explode(registry, dying.first, state, sheets, sounds);
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
