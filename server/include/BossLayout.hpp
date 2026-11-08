/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BossLayout
*/

#ifndef SERVER_BOSSLAYOUT_HPP
#define SERVER_BOSSLAYOUT_HPP

#include "engine/Vec2.hpp"

#include <cstddef>

namespace server {

// Where the parts of the bosses made of several entities are, relative to
// their boss, in playfield units. The client draws the behemoth at 2.5
// times its sprite size: its head sits on the opening of its belly.

/** @brief The size of the ceiling half of the twin. */
inline constexpr engine::Vec2f TwinTopSize{.x = 200.0F, .y = 210.0F};

/** @brief Where the head of the behemoth is. */
inline constexpr engine::Vec2f BehemothHeadOffset{.x = 26.0F, .y = 20.0F};
/** @brief The size of the head of the behemoth. */
inline constexpr engine::Vec2f BehemothHeadSize{.x = 80.0F, .y = 80.0F};
/** @brief The damage taken by the behemoth from a shot on its head, in times
 * the damage of the shot. */
inline constexpr float BehemothHeadDamage = 3.0F;
/** @brief Where the tail of the behemoth starts. */
inline constexpr engine::Vec2f BehemothTailBase{.x = -40.0F, .y = 215.0F};
/** @brief The number of segments of the tail, its tip included. */
inline constexpr std::size_t BehemothTailSegments = 8;
/** @brief The distance between two segments of the tail. */
inline constexpr float BehemothTailSpacing = 42.0F;
/** @brief How much more each segment of the tail waves than the previous
 * one. */
inline constexpr float BehemothTailSwing = 14.0F;
/** @brief The size of a segment of the tail. */
inline constexpr engine::Vec2f BehemothTailSize{.x = 44.0F, .y = 44.0F};
/** @brief The size of the tip of the tail. */
inline constexpr engine::Vec2f BehemothTailTipSize{.x = 60.0F, .y = 58.0F};
/** @brief The damage taken by the behemoth from a shot on its tail, in times
 * the damage of the shot: the tail is armored. */
inline constexpr float BehemothTailDamage = 0.5F;
/** @brief How fast the tail waves, in radians per second. */
inline constexpr float BehemothTailSpeed = 2.2F;

} // namespace server

#endif // SERVER_BOSSLAYOUT_HPP
