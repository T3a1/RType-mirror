/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** SnapshotHistory
*/

#ifndef COMMON_COMPONENTS_SNAPSHOTHISTORY_HPP
#define COMMON_COMPONENTS_SNAPSHOTHISTORY_HPP

#include "engine/Vec2.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace rtype {

/**
 * @struct SnapshotSample
 * @brief The state of an entity in one server snapshot.
 * @param tick The server tick of the snapshot.
 * @param position The position at that tick, in playfield units.
 * @param health The health points at that tick.
 **/
struct SnapshotSample {
    std::uint32_t tick = 0;
    engine::Vec2f position;
    int health = 0;
};

/**
 * @struct SnapshotHistory
 * @brief The last states the server sent for an entity, oldest first,
 * that the client interpolates between.
 * @param samples The samples; only the first count are used.
 * @param count The number of samples, at most Capacity.
 * @param lead How many ticks ahead of the render clock the entity is drawn:
 * 0 for most entities, InterpolationDelay to draw it in the present like the
 * local ship.
 **/
struct SnapshotHistory {
    static constexpr std::size_t Capacity = 8;
    std::array<SnapshotSample, Capacity> samples{};
    std::size_t count = 0;
    double lead = 0.0;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_SNAPSHOTHISTORY_HPP
