/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BossSystem
*/

#include "systems/BossSystem.hpp"
#include "EnemyDefs.hpp"
#include "SpawnRequests.hpp"
#include "common/Playfield.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/Health.hpp"
#include "common/components/Player.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "components/Boss.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"

#include <array>
#include <cmath>
#include <limits>
#include <numbers>
#include <optional>
#include <vector>

namespace server {

namespace {

// Where the boss stops, from the right edge of the playfield.
constexpr float StopFromRight = 320.0F;
// Space kept between the boss and the top and bottom edges.
constexpr float VerticalMargin = 20.0F;
constexpr float ShotSpeed = 420.0F;
constexpr float SpreadAngle = 0.25F; // radians between two shots

struct PhaseStats {
    float verticalSpeed;
    engine::Duration attackInterval;
    int spreadShots; // 1: a single aimed shot
};

constexpr std::array<PhaseStats, BossPhases> Phases{{
    {.verticalSpeed = 90.0F,
     .attackInterval = engine::Duration{1.1F},
     .spreadShots = 1},
    {.verticalSpeed = 130.0F,
     .attackInterval = engine::Duration{1.0F},
     .spreadShots = 3},
    {.verticalSpeed = 200.0F,
     .attackInterval = engine::Duration{0.7F},
     .spreadShots = 5},
}};

/**
 * @brief Finds the direction from a point to the nearest player.
 * @param registry The registry holding the players.
 * @param from The point.
 * @return The unit direction, or straight left if there is no player.
 **/
engine::Vec2f towardsNearestPlayer(engine::Registry &registry,
                                   engine::Vec2f from)
{
    std::optional<engine::Vec2f> nearest;
    float best = std::numeric_limits<float>::max();
    registry.each<rtype::Player, rtype::Transform>(
        [&](engine::Entity, const rtype::Player &,
            const rtype::Transform &transform) {
            const engine::Vec2f offset = transform.position - from;
            const float distance = std::hypot(offset.x, offset.y);
            if (distance > 0.0F && distance < best) {
                best = distance;
                nearest = offset * (1.0F / distance);
            }
        });
    return nearest.value_or(engine::Vec2f{.x = -1.0F, .y = 0.0F});
}

/**
 * @brief Builds the shots of one attack: a fan of spreadShots shots around
 * the aim, plus an aimed shot in phase 2.
 * @param boss The boss entity.
 * @param muzzle Where the shots start.
 * @param aim The unit direction to the nearest player.
 * @param phase The attack phase.
 * @return The shots.
 **/
std::vector<SpawnProjectile> attack(engine::Entity boss, engine::Vec2f muzzle,
                                    engine::Vec2f aim, std::uint8_t phase)
{
    const PhaseStats &stats = Phases.at(phase - 1U);
    const float center = std::atan2(aim.y, aim.x);
    std::vector<SpawnProjectile> shots;
    const auto shoot = [&](float angle) {
        shots.push_back({
            .owner = boss,
            .position = muzzle,
            .velocity = {.x = std::cos(angle) * ShotSpeed,
                         .y = std::sin(angle) * ShotSpeed},
            .fromPlayer = false,
        });
    };
    const float half = static_cast<float>(stats.spreadShots - 1) / 2.0F;
    for (int i = 0; i < stats.spreadShots; ++i) {
        shoot(center + ((static_cast<float>(i) - half) * SpreadAngle));
    }
    if (phase == 2) {
        // The middle of an even fan would miss: add the aimed shot.
        shoot(center + (SpreadAngle / 2.0F));
    }
    return shots;
}

} // namespace

std::uint8_t bossPhase(int health, int maxHealth)
{
    if (maxHealth <= 0 || health * 3 > maxHealth * 2) {
        return 1;
    }
    return health * 3 > maxHealth ? 2 : 3;
}

void bossSystem(engine::Registry &registry, SpawnCommands &commands,
                engine::Duration dt)
{
    const EnemyDef &stat = enemyDef(rtype::EnemyType::Boss);
    const float stopX = rtype::PlayfieldWidth - StopFromRight;
    const float top = (stat.size.y / 2.0F) + VerticalMargin;
    const float bottom = rtype::PlayfieldHeight - top;

    registry.each<Boss, rtype::Transform, rtype::Velocity, rtype::Health>(
        [&](engine::Entity entity, Boss &boss,
            const rtype::Transform &transform, rtype::Velocity &velocity,
            const rtype::Health &health) {
            const std::uint8_t phase = bossPhase(health.current, health.max);
            if (phase != boss.phase) {
                BB_INFO("Boss phase " << static_cast<int>(phase));
                boss.phase = phase;
            }
            const PhaseStats &stats = Phases.at(phase - 1U);
            if (!boss.arrived) {
                boss.arrived = transform.position.x <= stopX;
                velocity.value = {.x = boss.arrived ? 0.0F : -stat.speed,
                                  .y = 0.0F};
                return;
            }
            if (transform.position.y <= top) {
                boss.direction = 1.0F;
            } else if (transform.position.y >= bottom) {
                boss.direction = -1.0F;
            }
            velocity.value = {.x = 0.0F,
                              .y = boss.direction * stats.verticalSpeed};

            boss.cooldown -= dt;
            if (boss.cooldown.count() > 0.0F) {
                return;
            }
            boss.cooldown = stats.attackInterval;
            const engine::Vec2f muzzle{.x = transform.position.x -
                                            (stat.size.x / 2.0F),
                                       .y = transform.position.y};
            const auto attackShots = attack(
                entity, muzzle, towardsNearestPlayer(registry, muzzle), phase);
            commands.insert(commands.end(), attackShots.begin(),
                            attackShots.end());
        });
}

} // namespace server
