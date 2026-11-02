/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** InterpolationSystem
*/

#ifndef COMMON_SYSTEMS_INTERPOLATIONSYSTEM_HPP
#define COMMON_SYSTEMS_INTERPOLATIONSYSTEM_HPP

#include "common/components/SnapshotHistory.hpp"
#include "engine/Clock.hpp"
#include "engine/Registry.hpp"
#include "engine/Vec2.hpp"

#include <cstdint>

namespace rtype {

/**
 * The number of server ticks per second.
 **/
inline constexpr double TickRate = 60.0;

/**
 * How far in the past, in ticks, the client draws the synced entities: two
 * snapshots at 20 Hz, so that one lost snapshot still leaves two to
 * interpolate between.
 **/
inline constexpr double InterpolationDelay = 6.0;

/**
 * How long, in ticks, an entity keeps moving past its last snapshot before
 * it stops.
 **/
inline constexpr double MaxExtrapolation = 12.0;

/**
 * @struct RenderClock
 * @brief The server tick the client draws, InterpolationDelay ticks behind
 * the newest snapshot, advanced smoothly between snapshots.
 * @param tick The tick drawn, with a fractional part.
 * @param started Whether a snapshot was received yet.
 **/
struct RenderClock {
    double tick = 0.0;
    bool started = false;
};

/**
 * @brief Adds the state of a snapshot to an entity's history. A sample
 * older than the newest one is ignored, one at the same tick replaces it,
 * and the oldest sample is dropped when the history is full.
 * @param history The entity's history.
 * @param sample The entity's state in the snapshot.
 **/
void pushSample(SnapshotHistory &history, const SnapshotSample &sample);

/**
 * @brief Returns the position of an entity at a tick: interpolated between
 * the two samples around it, the oldest one before the history, or
 * extrapolated from the last two samples for at most MaxExtrapolation
 * ticks after it.
 * @param history The entity's history, with at least one sample.
 * @param tick The tick, with a fractional part.
 * @return The position at that tick.
 **/
engine::Vec2f samplePosition(const SnapshotHistory &history, double tick);

/**
 * @brief Returns the health of an entity at a tick: that of the newest
 * sample not after it, or of the oldest sample before the history. Health
 * is not interpolated, so that it drops when the hit is drawn.
 * @param history The entity's history, with at least one sample.
 * @param tick The tick, with a fractional part.
 * @return The health points at that tick.
 **/
int sampleHealth(const SnapshotHistory &history, double tick);

/**
 * @brief Advances the render clock by dt and pulls it toward
 * InterpolationDelay ticks behind the newest snapshot: it jumps there on the
 * first snapshot or when more than a second off, and catches up gradually
 * otherwise so that it never goes backwards.
 * @param clock The render clock.
 * @param newestTick The tick of the newest snapshot, 0 if none yet.
 * @param dt The time elapsed since the last frame.
 **/
void advanceRenderClock(RenderClock &clock, std::uint32_t newestTick,
                        engine::Duration dt);

/**
 * @brief Moves every entity with a Transform and a SnapshotHistory to its
 * position at the render tick plus its lead, and sets its Health, if it has
 * one, to its health at that tick.
 * @param registry The registry.
 * @param tick The render tick.
 **/
void interpolationSystem(engine::Registry &registry, double tick);

} // namespace rtype

#endif // COMMON_SYSTEMS_INTERPOLATIONSYSTEM_HPP
