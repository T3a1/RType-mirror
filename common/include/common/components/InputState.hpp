/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** InputState
*/

#ifndef COMMON_COMPONENTS_INPUTSTATE_HPP
#define COMMON_COMPONENTS_INPUTSTATE_HPP

#include <cstdint>

namespace rtype {

/** @brief The bits of InputState::actions. */
namespace Action {
/** @brief Moves the ship up. */
inline constexpr std::uint8_t Up = 1U << 0U;
/** @brief Moves the ship down. */
inline constexpr std::uint8_t Down = 1U << 1U;
/** @brief Moves the ship to the left. */
inline constexpr std::uint8_t Left = 1U << 2U;
/** @brief Moves the ship to the right. */
inline constexpr std::uint8_t Right = 1U << 3U;
/** @brief Fires normal shots while held: sent by auto-fire. */
inline constexpr std::uint8_t Fire = 1U << 4U;
/** @brief Charges a shot while held, fired when released. */
inline constexpr std::uint8_t Charge = 1U << 5U;
/** @brief Every bit above. */
inline constexpr std::uint8_t All = 0x3F;
} // namespace Action

/**
 * @struct InputState
 * @brief The actions a player holds during this tick, written from the network
 * input. Test one with (actions & Action::Fire) != 0.
 * @param actions The held actions, as Action bits.
 **/
struct InputState {
    std::uint8_t actions = 0;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_INPUTSTATE_HPP
