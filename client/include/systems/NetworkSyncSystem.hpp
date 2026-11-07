/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NetworkSyncSystem
*/

#ifndef CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP
#define CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP

#include "Chat.hpp"
#include "PlayerNotices.hpp"
#include "SpriteSheets.hpp"
#include "audio/SoundBoard.hpp"
#include "common/components/Prediction.hpp"
#include "common/protocol/Packet.hpp"
#include "common/systems/InterpolationSystem.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

namespace client {

/** @brief How long the banner of a new level is shown, in seconds: the
 * server's pause between two levels. */
inline constexpr float LevelBannerTime = 3.0F;

/**
 * @struct SyncState
 * @brief What the client knows about the server's world.
 * @param entities The local entity of each NetworkId.
 * @param destroyed Recently destroyed NetworkIds and the tick they were
 * destroyed at, so that a late snapshot cannot bring them back.
 * @param lastTick The newest snapshot tick applied.
 * @param localShip The NetworkId of the local player's ship, 0 until
 * Welcome.
 * @param localSlot The local player's slot, 0 until Welcome.
 * @param players The players block of the last snapshot.
 * @param clock The server tick the synced entities are drawn at.
 * @param dying Destroyed entities still drawn until the render clock
 * reaches the given tick: the tick they were destroyed at, minus their
 * lead.
 * @param interpolate Whether the other entities are interpolated; when
 * false they jump to each snapshot, as in the Part 1 prototype (benchmark
 * baseline).
 * @param predict Whether the local ship is predicted; when false it jumps
 * to each snapshot.
 * @param prediction The prediction of the local ship.
 * @param ackTick The tick of the last players block giving the local
 * player's last applied input.
 * @param ackInput That last applied input.
 * @param shipTick The tick of the last snapshot giving the local ship's
 * position.
 * @param shipPosition That position.
 * @param acknowledgedTick The tick last handed to the prediction, when
 * ackTick and shipTick matched.
 * @param gameOver Whether the server announced the end of the game.
 * @param started Whether the game of the room has started.
 * @param level The level announced last by GameEvent{LevelStarted}: from
 * 1, 0 for the endless mode or before any announce.
 * @param levelAnnounced Whether a level was announced since the game
 * started.
 * @param levelBanner How long the banner of that level is still shown, in
 * seconds.
 * @param notices The players who just joined or left, oldest first.
 * @param chat The chat of the room.
 **/
struct SyncState {
    std::unordered_map<std::uint32_t, engine::Entity> entities;
    std::unordered_map<std::uint32_t, std::uint32_t> destroyed;
    std::uint32_t lastTick = 0;
    std::uint32_t localShip = 0;
    std::uint8_t localSlot = 0;
    std::vector<rtype::protocol::PlayerInfo> players;
    rtype::RenderClock clock;
    std::vector<std::pair<engine::Entity, double>> dying;
    bool interpolate = true;
    bool predict = true;
    rtype::Prediction prediction;
    std::uint32_t ackTick = 0;
    std::uint32_t ackInput = 0;
    std::uint32_t shipTick = 0;
    engine::Vec2f shipPosition;
    std::uint32_t acknowledgedTick = 0;
    bool gameOver = false;
    bool started = false;
    std::uint8_t level = 0;
    bool levelAnnounced = false;
    float levelBanner = 0.0F;
    std::vector<PlayerNotice> notices;
    ChatState chat;
};

/**
 * @brief Applies what the server sent to the local registry. A snapshot
 * creates unknown entities (NetworkId, EntityType, Transform, Health, their
 * sprite (attachSprite()), and a SnapshotHistory except for the local ship)
 * and adds to the
 * history of known ones, or updates the Transform and Health of the local
 * ship;
 * snapshots older than the last one are ignored. With prediction, the local
 * ship's position and last applied input go to the prediction instead of its
 * Transform. EntityDestroyed hands the entity to the dying list; Welcome
 * marks the local ship. Runs first in the frame, on the payloads of
 * NetworkClient::update().
 * @param registry The client's registry.
 * @param payloads The payloads received this frame, in order.
 * @param state The sync state, updated.
 * @param sheets The sprite sheets the entities and explosions are drawn
 * with.
 * @param sounds The sounds played for what the server sent.
 **/
void networkSyncSystem(engine::Registry &registry,
                       const std::vector<rtype::protocol::Payload> &payloads,
                       SyncState &state, const SpriteSheets &sheets,
                       audio::SoundBoard &sounds);

/**
 * @brief Advances the render clock, moves the synced entities to their
 * interpolated position at its tick, and destroys the dying entities whose
 * tick it reached, leaving an explosion where ships and enemies were. Runs
 * right after networkSyncSystem().
 * @param registry The client's registry.
 * @param state The sync state.
 * @param dt The time elapsed since the last frame.
 * @param sheets The sprite sheets explosions are drawn with.
 * @param sounds The sounds played when an explosion appears.
 **/
void smoothingSystem(engine::Registry &registry, SyncState &state,
                     engine::Duration dt, const SpriteSheets &sheets,
                     audio::SoundBoard &sounds);

/**
 * @brief Predicts the local ship: records the input just sent, replays the
 * inputs the server has not applied yet on its last position, and moves the
 * ship's Transform there. Does nothing without prediction or before the
 * first snapshot. Runs right after NetworkClient::sendInput().
 * @param registry The client's registry.
 * @param state The sync state.
 * @param sequence The sequence of the input just sent, 0 if none.
 * @param actions The actions of that input.
 * @param dt The duration of the frame.
 **/
void predictionSystem(engine::Registry &registry, SyncState &state,
                      std::uint32_t sequence, std::uint8_t actions,
                      engine::Duration dt);

} // namespace client

#endif // CLIENT_SYSTEMS_NETWORKSYNCSYSTEM_HPP
