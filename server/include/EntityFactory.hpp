/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EntityFactory
*/

#ifndef SERVER_ENTITYFACTORY_HPP
#define SERVER_ENTITYFACTORY_HPP

#include "Difficulty.hpp"
#include "EnemyDefs.hpp"
#include "SpawnRequests.hpp"
#include "common/components/Collider.hpp"
#include "common/components/Damage.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/InputState.hpp"
#include "common/components/Lifetime.hpp"
#include "common/components/NetworkId.hpp"
#include "common/components/Player.hpp"
#include "common/components/Projectile.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "common/components/Weapon.hpp"
#include "components/Boss.hpp"
#include "components/DestroyOffScreen.hpp"
#include "components/KeepInPlayfield.hpp"
#include "components/Obstacle.hpp"
#include "components/Piercing.hpp"
#include "components/PowerUp.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"

#include <cstdint>

namespace server {

/**
 * @class EntityFactory
 * @brief Builds every gameplay entity of the server from a spawn request.
 *
 * Each create() overload assembles a complete entity in one call and gives
 * it a unique NetworkId, starting at 1 and never reused. Systems collect
 * their requests during their loop and call create() after it.
 **/
class EntityFactory {
  public:
    /**
     * @brief Sets the difficulty of the enemies created from now on: their
     * health and the time between their shots are scaled.
     * @param difficulty The factors of the difficulty.
     **/
    void setDifficulty(const DifficultyStats &difficulty)
    {
        m_difficulty = difficulty;
    }

    /**
     * @brief Creates an enemy with the stats of its kind in the enemy
     * catalog (the built-in kind of its type unless the request names one).
     *
     * The enemy moves left at its speed, collides with players and player
     * projectiles, is destroyed off-screen, and gets an unarmed Weapon if
     * its type shoots.
     * @param registry The registry to create the enemy in.
     * @param request The type and position of the enemy.
     * @return The created entity.
     **/
    engine::Entity create(engine::Registry &registry,
                          const SpawnEnemy &request);

    /**
     * @brief Creates a player ship.
     *
     * The ship has full health, an InputState written from the network, a
     * Weapon, a 64 x 64 collider and stays inside the playfield.
     * @param registry The registry to create the ship in.
     * @param request The slot and position of the player.
     * @return The created entity.
     **/
    engine::Entity create(engine::Registry &registry,
                          const SpawnPlayer &request);

    /**
     * @brief Creates a projectile.
     *
     * The projectile deals the damage of the request, lives at most 5
     * seconds and is destroyed off-screen. Its sprite and collision layers
     * depend on the side that fired it; a charged shot is larger and
     * Piercing.
     * @param registry The registry to create the projectile in.
     * @param request The owner, position, velocity and side of the
     * projectile.
     * @return The created entity.
     **/
    engine::Entity create(engine::Registry &registry,
                          const SpawnProjectile &request);

    /**
     * @brief Creates a power-up that floats to the left and is destroyed
     * off-screen; only players collide with it.
     * @param registry The registry to create the power-up in.
     * @param request Where it appears.
     * @return The created entity.
     **/
    engine::Entity create(engine::Registry &registry,
                          const SpawnPowerUp &request);

    /**
     * @brief Creates a wall block or a destroyable tile of TileSize that
     * scrolls to the left at ObstacleSpeed and is destroyed off-screen.
     *
     * It collides with the ships and every shot, and deals the Damage that
     * destroys a ship. A tile also gets TileMaxHealth.
     * @param registry The registry to create the block in.
     * @param request Where it appears and whether it is destroyable.
     * @return The created entity.
     **/
    engine::Entity create(engine::Registry &registry,
                          const SpawnObstacle &request);

  private:
    std::uint32_t m_nextNetworkId = 1;
    DifficultyStats m_difficulty;
};

} // namespace server

#endif // SERVER_ENTITYFACTORY_HPP
