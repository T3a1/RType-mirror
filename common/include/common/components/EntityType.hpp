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

/**
 * @enum EntityType
 * @brief What an entity is, as far as drawing it goes. Sent in snapshots: the
 * client picks the sprite from it. New values go at the end.
 * @param PlayerShip The ship of a player.
 * @param PlayerMissile A normal shot of a player.
 * @param EnemyBasic An EnemyType::Basic.
 * @param EnemyWave An EnemyType::Wave.
 * @param EnemyShooter An EnemyType::Shooter.
 * @param EnemyMissile A shot of an enemy.
 * @param EnemySmall An EnemyType::Small.
 * @param EnemyMedium An EnemyType::Medium.
 * @param EnemyLarge An EnemyType::Large.
 * @param Boss An EnemyType::Boss.
 * @param ChargedShot The piercing shot of a player who held the fire key.
 * @param PowerUp A pickup that raises the weapon level of a player.
 **/
enum class EntityType : std::uint8_t {
    PlayerShip,
    PlayerMissile,
    EnemyBasic,
    EnemyWave,
    EnemyShooter,
    EnemyMissile,
    EnemySmall,
    EnemyMedium,
    EnemyLarge,
    Boss,
    ChargedShot,
    PowerUp,
    BossHive,
};

// Whether an entity type is an end-level boss.
constexpr bool isBossEntity(EntityType type)
{
    return type == EntityType::Boss || type == EntityType::BossHive;
}

} // namespace rtype

#endif // COMMON_COMPONENTS_ENTITYTYPE_HPP
