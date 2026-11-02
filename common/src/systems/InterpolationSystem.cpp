/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** InterpolationSystem
*/

#include "common/systems/InterpolationSystem.hpp"
#include "common/components/Health.hpp"
#include "common/components/Transform.hpp"
#include "engine/Entity.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>

namespace rtype {

namespace {

constexpr double ResyncThreshold = TickRate;
constexpr double CatchUpRate = 2.0;

/**
 * @brief Interpolates linearly between two samples.
 * @param from The earlier sample.
 * @param to The later sample, at a later tick.
 * @param tick The tick, between the two samples' ticks.
 * @return The position at that tick.
 **/
engine::Vec2f lerp(const SnapshotSample &from, const SnapshotSample &to,
                   double tick)
{
    const auto alpha = static_cast<float>(
        (tick - from.tick) / static_cast<double>(to.tick - from.tick));
    return from.position + (to.position - from.position) * alpha;
}

} // namespace

void pushSample(SnapshotHistory &history, const SnapshotSample &sample)
{
    if (history.count > 0) {
        SnapshotSample &newest = history.samples.at(history.count - 1);
        if (sample.tick < newest.tick) {
            return;
        }
        if (sample.tick == newest.tick) {
            newest = sample;
            return;
        }
    }
    if (history.count == SnapshotHistory::Capacity) {
        std::ranges::rotate(history.samples,
                            std::next(history.samples.begin()));
        --history.count;
    }
    history.samples.at(history.count) = sample;
    ++history.count;
}

engine::Vec2f samplePosition(const SnapshotHistory &history, double tick)
{
    const SnapshotSample &oldest = history.samples.front();
    if (history.count == 1 || tick <= oldest.tick) {
        return oldest.position;
    }
    const SnapshotSample &newest = history.samples.at(history.count - 1);
    if (tick >= newest.tick) {
        const SnapshotSample &previous = history.samples.at(history.count - 2);
        return lerp(previous, newest,
                    std::min(tick, newest.tick + MaxExtrapolation));
    }
    std::size_t next = 1;
    while (history.samples.at(next).tick < tick) {
        ++next;
    }
    return lerp(history.samples.at(next - 1), history.samples.at(next), tick);
}

int sampleHealth(const SnapshotHistory &history, double tick)
{
    std::size_t index = 0;
    while (index + 1 < history.count &&
           history.samples.at(index + 1).tick <= tick) {
        ++index;
    }
    return history.samples.at(index).health;
}

void advanceRenderClock(RenderClock &clock, std::uint32_t newestTick,
                        engine::Duration dt)
{
    if (newestTick == 0) {
        return;
    }
    const double target = newestTick - InterpolationDelay;
    clock.tick += dt.count() * TickRate;
    const double error = target - clock.tick;
    if (!clock.started || std::abs(error) > ResyncThreshold) {
        clock.tick = target;
        clock.started = true;
        return;
    }
    clock.tick += error * std::min(1.0, dt.count() * CatchUpRate);
}

void interpolationSystem(engine::Registry &registry, double tick)
{
    registry.each<SnapshotHistory, Transform>(
        [tick](engine::Entity, SnapshotHistory &history, Transform &transform) {
            if (history.count > 0) {
                transform.position =
                    samplePosition(history, tick + history.lead);
            }
        });
    registry.each<SnapshotHistory, Health>(
        [tick](engine::Entity, SnapshotHistory &history, Health &health) {
            if (history.count > 0) {
                health.current = sampleHealth(history, tick + history.lead);
            }
        });
}

} // namespace rtype
