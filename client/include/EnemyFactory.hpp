/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** EnemyFactory
*/

#ifndef CLIENT_ENEMYFACTORY_HPP
#define CLIENT_ENEMYFACTORY_HPP

#include "common/components/EntityType.hpp"
#include "components/Sprite.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "engine/Vec2.hpp"

#include <optional>

namespace client {

/**
 * @brief Returns how an enemy of the given type is drawn.
 *
 * EnemySmall is a 50x50 red square, EnemyMedium a 100x50 blue rectangle
 * and EnemyLarge a 100x100 green square.
 *
 * @param type Type of the enemy.
 * @return Its sprite, or std::nullopt if the type is not an enemy.
 */
std::optional<Sprite> enemySprite(rtype::EntityType type);

/**
 * @brief Creates an enemy just behind the right edge of the window.
 *
 * The enemy gets a Transform, a Velocity, a Sprite and its EntityType, so the
 * movement and render systems handle it like any other entity. For now the
 * arguments come from a debug key; later they will come from the server.
 *
 * @param registry Registry to create the enemy in.
 * @param y Vertical position of the enemy's center, in pixels.
 * @param velocity Speed of the enemy in pixels per second.
 * @param type Type of the enemy, which picks its sprite.
 * @return The created entity, or std::nullopt if the type is not an enemy.
 */
std::optional<engine::Entity> spawnEnemy(engine::Registry &registry, float y,
                                         engine::Vec2f velocity,
                                         rtype::EntityType type);

} // namespace client

#endif // CLIENT_ENEMYFACTORY_HPP
