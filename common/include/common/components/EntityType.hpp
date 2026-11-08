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
 * @param BossHive An EnemyType::Hive, the second end-level boss.
 * @param Obstacle A wall of the level: it stops every shot and destroys the
 * ships that touch it.
 * @param DestroyableTile A block of the level that stops shots like a wall
 * but breaks after a few hits.
 * @param EnemySpinner An EnemyType::Spinner.
 * @param EnemyBubble An EnemyType::Bubble.
 * @param EnemySpore An EnemyType::Spore.
 * @param EnemyJelly An EnemyType::Jelly.
 * @param EnemyCrab An EnemyType::Crab.
 * @param BossWalker An EnemyType::Walker, the boss walking on the ground.
 * @param BossTwin An EnemyType::Twin: the half of the twin boss on the
 * ground.
 * @param BossTwinTop The half of the twin boss on the ceiling.
 * @param BossBehemoth An EnemyType::Behemoth: the body of the last boss.
 * @param BehemothHead The head of the behemoth, its weak point.
 * @param BehemothTail A segment of the behemoth's tail.
 * @param BehemothTailTip The end of the behemoth's tail.
 * @param BossFireball A shot of the walker.
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
    Obstacle,
    DestroyableTile,
    EnemySpinner,
    EnemyBubble,
    EnemySpore,
    EnemyJelly,
    EnemyCrab,
    BossWalker,
    BossTwin,
    BossTwinTop,
    BossBehemoth,
    BehemothHead,
    BehemothTail,
    BehemothTailTip,
    BossFireball,
};

/** @brief The last EntityType, for the checks of the protocol. */
inline constexpr EntityType LastEntityType = EntityType::BossFireball;

/**
 * @brief Tells whether an entity type is an end-level boss: the entity that
 * holds its health, not one of its other parts.
 * @param type The entity type.
 * @return `true` for a boss.
 **/
constexpr bool isBossEntity(EntityType type)
{
    return type == EntityType::Boss || type == EntityType::BossHive ||
           type == EntityType::BossWalker || type == EntityType::BossTwin ||
           type == EntityType::BossBehemoth;
}

/**
 * @brief Tells whether an entity type is a part of the level.
 * @param type The entity type.
 * @return `true` for a wall or a tile.
 **/
constexpr bool isObstacleEntity(EntityType type)
{
    return type == EntityType::Obstacle || type == EntityType::DestroyableTile;
}

} // namespace rtype

#endif // COMMON_COMPONENTS_ENTITYTYPE_HPP
