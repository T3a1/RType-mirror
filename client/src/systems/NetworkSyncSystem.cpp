/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NetworkSyncSystem
*/

#include "systems/NetworkSyncSystem.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/NetworkId.hpp"
#include "common/components/Player.hpp"
#include "common/components/Transform.hpp"
#include "components/Sprite.hpp"
#include "engine/Logger.hpp"

#include <variant>

namespace client {

namespace {

using namespace rtype::protocol;

constexpr std::uint32_t DestroyedMemory = 300;

constexpr Color rgb(unsigned char red, unsigned char green, unsigned char blue)
{
    return {.r = red, .g = green, .b = blue, .a = 255};
}

constexpr engine::Vec2f ShipSize{.x = 64.0F, .y = 32.0F};
constexpr engine::Vec2f EnemySize{.x = 64.0F, .y = 64.0F};
constexpr engine::Vec2f MissileSize{.x = 16.0F, .y = 8.0F};

Sprite spriteOf(rtype::EntityType type, bool local)
{
    switch (type) {
    case rtype::EntityType::PlayerShip:
        return {
            .size = ShipSize,
            .tint = local ? rgb(80, 160, 255) : rgb(120, 200, 120),
            .layer = 2,
        };
    case rtype::EntityType::PlayerMissile:
        return {.size = MissileSize, .tint = rgb(255, 220, 60), .layer = 1};
    case rtype::EntityType::EnemyBasic:
        return {.size = EnemySize, .tint = rgb(230, 60, 60), .layer = 1};
    case rtype::EntityType::EnemyWave:
        return {.size = EnemySize, .tint = rgb(240, 140, 40), .layer = 1};
    case rtype::EntityType::EnemyShooter:
        return {.size = EnemySize, .tint = rgb(170, 80, 220), .layer = 1};
    case rtype::EntityType::EnemyMissile:
        return {.size = MissileSize, .tint = rgb(255, 100, 180), .layer = 1};
    }
    return {.size = EnemySize};
}

void applyEntity(engine::Registry &registry, const EntityState &entity,
                 SyncState &state)
{
    if (state.destroyed.contains(entity.networkId)) {
        return;
    }
    const auto it = state.entities.find(entity.networkId);
    if (it != state.entities.end()) {
        registry.get<rtype::Transform>(it->second).position = {
            .x = entity.x,
            .y = entity.y,
        };
        registry.get<rtype::Health>(it->second).current = entity.health;
        return;
    }
    const engine::Entity created = registry.create();
    registry.emplace<rtype::NetworkId>(created,
                                       rtype::NetworkId{entity.networkId});
    registry.emplace<rtype::EntityType>(created, entity.type);
    registry.emplace<rtype::Transform>(
        created, rtype::Transform{.position = {.x = entity.x, .y = entity.y}});
    registry.emplace<rtype::Health>(created, rtype::Health{
                                                 .current = entity.health,
                                                 .max = rtype::PlayerMaxHealth,
                                             });
    registry.emplace<Sprite>(
        created, spriteOf(entity.type, entity.networkId == state.localShip));
    state.entities.emplace(entity.networkId, created);
}

void applySnapshot(engine::Registry &registry, const SnapshotPart &snapshot,
                   SyncState &state)
{
    if (snapshot.tick < state.lastTick) {
        return;
    }
    state.lastTick = snapshot.tick;
    if (snapshot.part == 0) {
        state.players = snapshot.players;
    }
    for (const EntityState &entity : snapshot.entities) {
        applyEntity(registry, entity, state);
    }
    std::erase_if(state.destroyed, [&state](const auto &entry) {
        return state.lastTick - entry.second > DestroyedMemory;
    });
}

void applyEvent(engine::Registry &registry, const GameEvent &event,
                SyncState &state)
{
    switch (event.kind) {
    case GameEventKind::EntityDestroyed: {
        state.destroyed[event.networkId] = state.lastTick;
        const auto it = state.entities.find(event.networkId);
        if (it != state.entities.end()) {
            registry.destroy(it->second);
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
    case GameEventKind::GameOver:
        break;
    }
}

void applyWelcome(engine::Registry &registry, const Welcome &welcome,
                  SyncState &state)
{
    BB_INFO("Playing as player " << static_cast<int>(welcome.slot));
    state.localShip = welcome.shipId;
    state.localSlot = welcome.slot;
    const auto it = state.entities.find(welcome.shipId);
    if (it != state.entities.end()) {
        registry.get<Sprite>(it->second) =
            spriteOf(rtype::EntityType::PlayerShip, true);
    }
}

} // namespace

void networkSyncSystem(engine::Registry &registry,
                       const std::vector<Payload> &payloads, SyncState &state)
{
    for (const Payload &payload : payloads) {
        if (const auto *snapshot = std::get_if<SnapshotPart>(&payload)) {
            applySnapshot(registry, *snapshot, state);
        } else if (const auto *event = std::get_if<GameEvent>(&payload)) {
            applyEvent(registry, *event, state);
        } else if (const auto *welcome = std::get_if<Welcome>(&payload)) {
            applyWelcome(registry, *welcome, state);
        }
    }
}

} // namespace client
