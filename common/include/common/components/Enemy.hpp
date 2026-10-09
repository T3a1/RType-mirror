/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** Enemy
*/

#ifndef COMMON_COMPONENTS_ENEMY_HPP
#define COMMON_COMPONENTS_ENEMY_HPP

#include "engine/Clock.hpp"

#include <cstdint>

namespace rtype {

enum class EnemyType : std::uint8_t {
    Basic,
    Wave,
    Shooter,
};

// Behavior of an enemy, read by the AI system.
struct Enemy {
    EnemyType type = EnemyType::Basic;
    // Time between two shots, and time left before the next one.
    engine::Duration fireInterval{2.0F};
    engine::Duration fireCooldown{0.0F};
};

} // namespace rtype

#endif // COMMON_COMPONENTS_ENEMY_HPP
