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

// Bits of InputState::actions.
namespace Action {
inline constexpr std::uint8_t Up = 1U << 0U;
inline constexpr std::uint8_t Down = 1U << 1U;
inline constexpr std::uint8_t Left = 1U << 2U;
inline constexpr std::uint8_t Right = 1U << 3U;
inline constexpr std::uint8_t Fire = 1U << 4U;
} // namespace Action

// Actions held by a player during this tick, written from the network
// input. Test one with (actions & Action::Fire) != 0.
struct InputState {
    std::uint8_t actions = 0;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_INPUTSTATE_HPP
