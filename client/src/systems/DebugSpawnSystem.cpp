/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** DebugSpawnSystem
*/

#include "systems/DebugSpawnSystem.hpp"
#include "EnemyFactory.hpp"
#include "Window.hpp"
#include "common/components/EntityType.hpp"

#include <raylib.h>

#include <array>
#include <cstddef>

namespace client {

namespace {

constexpr std::array enemyTypes{
    rtype::EntityType::EnemySmall,
    rtype::EntityType::EnemyMedium,
    rtype::EntityType::EnemyLarge,
};

constexpr engine::Vec2f enemyVelocity{.x = -200.0F, .y = 0.0F};

} // namespace

void debugSpawnSystem(engine::Registry &registry, std::mt19937 &random)
{
    if (!IsKeyPressed(KEY_SPACE)) {
        return;
    }

    std::uniform_int_distribution<std::size_t> pickType{0,
                                                        enemyTypes.size() - 1};
    const rtype::EntityType type = enemyTypes.at(pickType(random));
    const std::optional<Sprite> sprite = enemySprite(type);
    if (!sprite) {
        return;
    }

    const float halfHeight = sprite->size.y / 2.0F;
    std::uniform_real_distribution<float> pickY{
        halfHeight, static_cast<float>(windowHeight) - halfHeight};
    spawnEnemy(registry, pickY(random), enemyVelocity, type);
}

} // namespace client
