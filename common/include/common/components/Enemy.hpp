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

/**
 * @enum EnemyType
 * @brief The kinds of enemies, in the order of the server's ENEMY_DEFS.
 * @param Basic An enemy flying straight, which does not shoot.
 * @param Wave A fast enemy following a sine wave, which does not shoot.
 * @param Shooter An enemy flying straight and shooting.
 * @param Small The smallest and fastest enemy, following a sine wave.
 * @param Medium A shooting enemy following a zigzag, drops a power-up.
 * @param Large A slow, tough and shooting enemy, drops a power-up.
 * @param Boss The first end-level boss, moved by bossSystem().
 * @param Hive The second end-level boss, moved by bossSystem().
 **/
enum class EnemyType : std::uint8_t {
    Basic,
    Wave,
    Shooter,
    Small,
    Medium,
    Large,
    Boss,
    Hive,
};

/**
 * @brief Tells whether an enemy type is an end-level boss, moved by the boss
 * system.
 * @param type The enemy type.
 * @return `true` for a boss.
 **/
constexpr bool isBoss(EnemyType type)
{
    return type == EnemyType::Boss || type == EnemyType::Hive;
}

/** @brief Health of the end-level boss; the clients draw its health bar from
 * it. */
inline constexpr int BossMaxHealth = 2500;

/**
 * @struct Enemy
 * @brief The behavior of an enemy, read by the AI system.
 * @param type The built-in type whose behavior it has.
 * @param fireInterval The time between two shots, in seconds.
 * @param fireCooldown The time left before the next shot, in seconds.
 * @param kind Its id in the server's enemy catalog, which holds its stats; a
 * built-in kind has the id of its type.
 **/
struct Enemy {
    EnemyType type = EnemyType::Basic;
    engine::Duration fireInterval{2.0F};
    engine::Duration fireCooldown{0.0F};
    std::uint16_t kind = 0;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_ENEMY_HPP
