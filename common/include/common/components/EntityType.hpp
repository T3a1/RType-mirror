/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** EntityType
*/

#ifndef COMMON_COMPONENTS_ENTITYTYPE_HPP
#define COMMON_COMPONENTS_ENTITYTYPE_HPP

#include <cstdint>

namespace rtype {

// What an entity is, as far as drawing it goes. Sent in snapshots: the
// client picks the sprite from it. New values go at the end.
enum class EntityType : std::uint8_t {
    PlayerShip,
    PlayerMissile,
    EnemyBasic,
    EnemyWave,
    EnemyShooter,
    EnemyMissile,
};

} // namespace rtype

#endif // COMMON_COMPONENTS_ENTITYTYPE_HPP
