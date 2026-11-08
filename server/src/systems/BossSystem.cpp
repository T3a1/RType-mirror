/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BossSystem
*/

#include "systems/BossSystem.hpp"
#include "BossLayout.hpp"
#include "EnemyDefs.hpp"
#include "SpawnRequests.hpp"
#include "common/Playfield.hpp"
#include "common/components/Enemy.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/Player.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "components/Boss.hpp"
#include "components/BossPart.hpp"
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

/** @brief Where the boss stops, from the right edge of the playfield, in
 * playfield units. */
constexpr float StopFromRight = 320.0F;
/** @brief The space kept between the boss and the top and bottom edges, in
 * playfield units. */
constexpr float VerticalMargin = 20.0F;
/** @brief The speed of the boss's shots, in playfield units per second. */
constexpr float ShotSpeed = 420.0F;
/** @brief The angle between two shots of a fan, in radians. */
constexpr float SpreadAngle = 0.25F;

/**
 * @struct PhaseStats
 * @brief How the boss behaves in one phase.
 * @param verticalSpeed Its speed up and down, in playfield units per second.
 * @param attackInterval The time between two attacks, in seconds.
 * @param spreadShots The number of shots of a fan; 1 is a single aimed shot.
 **/
struct PhaseStats {
    float verticalSpeed;
    engine::Duration attackInterval;
    int spreadShots;
};

/** @brief The stats of each phase, the last ones at low health. */
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

/** @brief Half a turn, in radians. */
constexpr float Pi = std::numbers::pi_v<float>;
/** @brief Straight up, in radians (y points down). */
constexpr float Up = 1.5F * Pi;
/** @brief The leftmost point the walker walks to, in playfield units. */
constexpr float WalkerLeftmost = rtype::PlayfieldWidth * 0.3F;
/** @brief How far the behemoth moves up and down from the middle of the
 * playfield, in playfield units. */
constexpr float BehemothRange = 140.0F;

/** @brief The speed of the walker in each phase, in playfield units per
 * second. */
constexpr std::array<float, BossPhases> WalkerSpeeds{90.0F, 120.0F, 170.0F};
/** @brief The vertical speed of the behemoth in each phase. */
constexpr std::array<float, BossPhases> BehemothSpeeds{50.0F, 70.0F, 100.0F};

/** @brief The shots of one attack. */
using Shots = std::vector<SpawnProjectile>;

/**
 * @brief Adds one enemy shot.
 * @param shots The shots of the attack.
 * @param boss The boss firing it.
 * @param from Where it starts.
 * @param angle Its direction, in radians (y points down).
 * @param speed Its speed, in playfield units per second.
 * @param look What the clients draw for it.
 **/
void shoot(Shots &shots, engine::Entity boss, engine::Vec2f from, float angle,
           float speed,
           rtype::EntityType look = rtype::EntityType::EnemyMissile)
{
    shots.push_back({
        .owner = boss,
        .position = from,
        .velocity = {.x = std::cos(angle) * speed,
                     .y = std::sin(angle) * speed},
        .fromPlayer = false,
        .look = look,
    });
}

/**
 * @brief Adds a fan of shots.
 * @param shots The shots of the attack.
 * @param boss The boss firing them.
 * @param from Where they start.
 * @param center The direction of the middle of the fan, in radians.
 * @param count The number of shots.
 * @param step The angle between two shots, in radians.
 * @param speed Their speed, in playfield units per second.
 * @param look What the clients draw for them.
 **/
void fan(Shots &shots, engine::Entity boss, engine::Vec2f from, float center,
         int count, float step, float speed,
         rtype::EntityType look = rtype::EntityType::EnemyMissile)
{
    const float half = static_cast<float>(count - 1) / 2.0F;
    for (int i = 0; i < count; ++i) {
        shoot(shots, boss, from,
              center + ((static_cast<float>(i) - half) * step), speed, look);
    }
}

/**
 * @brief Adds a ring of shots in every direction.
 * @param shots The shots of the attack.
 * @param boss The boss firing them.
 * @param from Where they start.
 * @param count The number of shots.
 * @param turn The direction of the first shot, in radians.
 * @param speed Their speed, in playfield units per second.
 **/
void ring(Shots &shots, engine::Entity boss, engine::Vec2f from, int count,
          float turn, float speed)
{
    for (int i = 0; i < count; ++i) {
        shoot(shots, boss, from,
              turn + (2.0F * Pi * static_cast<float>(i) /
                      static_cast<float>(count)),
              speed);
    }
}

/**
 * @brief Returns the direction from a point to the nearest player.
 * @param registry The registry holding the players.
 * @param from The point.
 * @return The angle, in radians; straight left if there is no player.
 **/
float aimFrom(engine::Registry &registry, engine::Vec2f from)
{
    const engine::Vec2f aim = towardsNearestPlayer(registry, from);
    return std::atan2(aim.y, aim.x);
}

/**
 * @brief The attacks of the walker, with its fireballs: an aimed fan and an
 * arc over it in turns (phase 1), a stream sweeping the sky with an aimed
 * fan from time to time (phase 2), two streams and large arcs (phase 3).
 * @param registry The registry holding the players.
 * @param entity The walker.
 * @param boss Its state; the time before its next attack is set.
 * @param position Its position.
 * @param stat Its stats.
 * @return The shots.
 **/
Shots walkerAttack(engine::Registry &registry, engine::Entity entity,
                   Boss &boss, engine::Vec2f position, const EnemyDef &stat)
{
    constexpr auto Fireball = rtype::EntityType::BossFireball;
    const engine::Vec2f muzzle{.x = position.x +
                                    (boss.walk * stat.size.x * 0.4F),
                               .y = position.y - (stat.size.y * 0.25F)};
    const float aim = aimFrom(registry, muzzle);
    Shots shots;
    ++boss.volleys;
    if (boss.phase == 1) {
        boss.cooldown = engine::Duration{1.3F};
        if (boss.volleys % 2 == 0) {
            fan(shots, entity, muzzle, aim, 5, 0.2F, 420.0F, Fireball);
        } else {
            const float shift = boss.volleys % 4 == 1 ? 0.5F : 0.0F;
            for (int i = 0; i < 9; ++i) {
                shoot(shots, entity, muzzle,
                      Pi + ((static_cast<float>(i) + shift) * Pi / 9.0F),
                      360.0F, Fireball);
            }
        }
    } else if (boss.phase == 2) {
        boss.cooldown = engine::Duration{0.14F};
        boss.spin += 0.22F;
        shoot(shots, entity, muzzle, Up + (std::sin(boss.spin) * 0.47F * Pi),
              450.0F, Fireball);
        if (boss.volleys % 10 == 0) {
            fan(shots, entity, muzzle, aim, 3, 0.15F, 520.0F, Fireball);
        }
    } else {
        boss.cooldown = engine::Duration{0.11F};
        boss.spin += 0.27F;
        const float sweep = std::sin(boss.spin) * 0.47F * Pi;
        shoot(shots, entity, muzzle, Up + sweep, 470.0F, Fireball);
        shoot(shots, entity, muzzle, Up - sweep, 470.0F, Fireball);
        if (boss.volleys % 14 == 0) {
            for (int i = 0; i < 13; ++i) {
                shoot(shots, entity, muzzle,
                      Pi + ((static_cast<float>(i) + 0.5F) * Pi / 13.0F),
                      380.0F, Fireball);
            }
        }
    }
    return shots;
}

/**
 * @brief The attacks of the twin, from its two halves: aimed fans in turns
 * (phase 1), two streams sweeping like scissors (phase 2), shifting walls
 * of shots with aimed fans (phase 3).
 * @param registry The registry holding the players.
 * @param entity The twin.
 * @param boss Its state; the time before its next attack is set.
 * @param position The position of its ground half.
 * @param stat Its stats.
 * @return The shots.
 **/
Shots twinAttack(engine::Registry &registry, engine::Entity entity, Boss &boss,
                 engine::Vec2f position, const EnemyDef &stat)
{
    const engine::Vec2f top{.x = position.x - (stat.size.x * 0.2F),
                            .y = TwinTopSize.y};
    const engine::Vec2f bottom{.x = position.x - (stat.size.x * 0.2F),
                               .y = rtype::PlayfieldHeight - stat.size.y};
    Shots shots;
    ++boss.volleys;
    if (boss.phase == 1) {
        boss.cooldown = engine::Duration{1.0F};
        const engine::Vec2f from = boss.volleys % 2 == 0 ? top : bottom;
        fan(shots, entity, from, aimFrom(registry, from), 3, 0.22F, 430.0F);
    } else if (boss.phase == 2) {
        boss.cooldown = engine::Duration{0.16F};
        boss.spin += 0.2F;
        const float sweep = std::sin(boss.spin) * 0.22F * Pi;
        shoot(shots, entity, top, (0.75F * Pi) + sweep, 460.0F);
        shoot(shots, entity, bottom, (1.25F * Pi) - sweep, 460.0F);
        if (boss.volleys % 12 == 0) {
            shoot(shots, entity, top, aimFrom(registry, top), 540.0F);
            shoot(shots, entity, bottom, aimFrom(registry, bottom), 540.0F);
        }
    } else {
        boss.cooldown = engine::Duration{0.55F};
        constexpr float Step = 0.2F;
        const float shift = (boss.volleys % 2 == 0 ? 0.5F : -0.5F) * Step;
        fan(shots, entity, top, (0.8F * Pi) + shift, 7, Step, 380.0F);
        fan(shots, entity, bottom, (1.2F * Pi) - shift, 7, Step, 380.0F);
        if (boss.volleys % 3 == 0) {
            const engine::Vec2f from = boss.volleys % 2 == 0 ? top : bottom;
            fan(shots, entity, from, aimFrom(registry, from), 3, 0.12F, 560.0F);
        }
    }
    return shots;
}

/**
 * @brief Finds where the tip of a behemoth's tail is.
 * @param registry The registry holding the behemoth's parts.
 * @param entity The behemoth.
 * @param position Its position, for a behemoth without a tail.
 * @return The position of the tip.
 **/
engine::Vec2f tailTip(engine::Registry &registry, engine::Entity entity,
                      engine::Vec2f position)
{
    std::optional<engine::Vec2f> tip;
    registry.each<BossPart, rtype::EntityType, rtype::Transform>(
        [&](engine::Entity, const BossPart &part, const rtype::EntityType &type,
            const rtype::Transform &transform) {
            if (part.owner == entity &&
                type == rtype::EntityType::BehemothTailTip) {
                tip = transform.position;
            }
        });
    return tip.value_or(position + BehemothTailBase);
}

/**
 * @brief The attacks of the behemoth: bursts of aimed shots from its head
 * (phase 1), wider bursts and rings from the tip of its tail (phase 2), a
 * spiral from its head with rings and aimed shots (phase 3).
 * @param registry The registry holding the players and its tail.
 * @param entity The behemoth.
 * @param boss Its state; the time before its next attack is set.
 * @param position Its position.
 * @return The shots.
 **/
Shots behemothAttack(engine::Registry &registry, engine::Entity entity,
                     Boss &boss, engine::Vec2f position)
{
    const engine::Vec2f head = position + BehemothHeadOffset;
    const float aim = aimFrom(registry, head);
    Shots shots;
    ++boss.volleys;
    if (boss.phase < 3) {
        const bool pause = boss.volleys % 4 == 0;
        boss.cooldown = engine::Duration{pause ? 1.1F : 0.18F};
        if (!pause) {
            fan(shots, entity, head, aim, boss.phase == 1 ? 1 : 3, 0.12F,
                520.0F);
        }
        if (boss.phase == 2 && boss.volleys % 6 == 0) {
            boss.spin += 0.3F;
            ring(shots, entity, tailTip(registry, entity, position), 10,
                 boss.spin, 330.0F);
        }
        return shots;
    }
    boss.cooldown = engine::Duration{0.12F};
    boss.spin += 0.42F;
    shoot(shots, entity, head, boss.spin, 360.0F);
    shoot(shots, entity, head, boss.spin + Pi, 360.0F);
    if (boss.volleys % 8 == 0) {
        ring(shots, entity, tailTip(registry, entity, position), 14, -boss.spin,
             330.0F);
    }
    if (boss.volleys % 5 == 0) {
        shoot(shots, entity, head, aim, 560.0F);
    }
    return shots;
}

/**
 * @brief Tells whether a boss stands on the ground.
 * @param type Its type.
 * @return `true` for the hive, the walker and the twin.
 **/
bool grounded(rtype::EnemyType type)
{
    return type == rtype::EnemyType::Hive || type == rtype::EnemyType::Walker ||
           type == rtype::EnemyType::Twin;
}

/**
 * @brief Moves an arrived boss: the walker walks left and right, the twin
 * and the hive stay still, the others move up and down.
 * @param type Its type.
 * @param boss Its state.
 * @param position Its position.
 * @param velocity Its velocity, set.
 * @param stat Its stats.
 **/
void move(rtype::EnemyType type, Boss &boss, engine::Vec2f position,
          rtype::Velocity &velocity, const EnemyDef &stat)
{
    const std::size_t phase = boss.phase - 1U;
    if (type == rtype::EnemyType::Walker) {
        const float rightmost =
            rtype::PlayfieldWidth - (stat.size.x / 2.0F) - VerticalMargin;
        if (position.x <= WalkerLeftmost) {
            boss.walk = 1.0F;
        } else if (position.x >= rightmost) {
            boss.walk = -1.0F;
        }
        velocity.value = {.x = boss.walk * WalkerSpeeds.at(phase), .y = 0.0F};
        return;
    }
    if (grounded(type)) {
        velocity.value = {};
        return;
    }
    float top = (stat.size.y / 2.0F) + VerticalMargin;
    float bottom = rtype::PlayfieldHeight - top;
    float speed = Phases.at(phase).verticalSpeed;
    if (type == rtype::EnemyType::Behemoth) {
        top = (rtype::PlayfieldHeight / 2.0F) - BehemothRange;
        bottom = (rtype::PlayfieldHeight / 2.0F) + BehemothRange;
        speed = BehemothSpeeds.at(phase);
    }
    if (position.y <= top) {
        boss.direction = 1.0F;
    } else if (position.y >= bottom) {
        boss.direction = -1.0F;
    }
    velocity.value = {.x = 0.0F, .y = boss.direction * speed};
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
    const float stopX = rtype::PlayfieldWidth - StopFromRight;

    registry.each<Boss, rtype::Enemy, rtype::Transform, rtype::Velocity,
                  rtype::Health>([&](engine::Entity entity, Boss &boss,
                                     const rtype::Enemy &enemy,
                                     rtype::Transform &transform,
                                     rtype::Velocity &velocity,
                                     const rtype::Health &health) {
        const EnemyDef &stat = enemyDef(enemy);
        const std::uint8_t phase = bossPhase(health.current, health.max);
        if (phase != boss.phase) {
            BB_INFO("Boss phase " << static_cast<int>(phase));
            boss.phase = phase;
        }
        if (grounded(enemy.type)) {
            transform.position.y =
                rtype::PlayfieldHeight - (stat.size.y / 2.0F);
        }
        if (!boss.arrived) {
            boss.arrived = transform.position.x <= stopX;
            velocity.value = {.x = boss.arrived ? 0.0F : -stat.speed,
                              .y = 0.0F};
            return;
        }
        move(enemy.type, boss, transform.position, velocity, stat);

        boss.cooldown -= dt;
        if (boss.cooldown.count() > 0.0F) {
            return;
        }
        Shots shots;
        switch (enemy.type) {
        case rtype::EnemyType::Walker:
            shots =
                walkerAttack(registry, entity, boss, transform.position, stat);
            break;
        case rtype::EnemyType::Twin:
            shots =
                twinAttack(registry, entity, boss, transform.position, stat);
            break;
        case rtype::EnemyType::Behemoth:
            shots = behemothAttack(registry, entity, boss, transform.position);
            break;
        default: {
            boss.cooldown = Phases.at(phase - 1U).attackInterval;
            const engine::Vec2f muzzle{.x = transform.position.x -
                                            (stat.size.x / 2.0F),
                                       .y = transform.position.y};
            shots = attack(entity, muzzle,
                           towardsNearestPlayer(registry, muzzle), phase);
            break;
        }
        }
        commands.insert(commands.end(), shots.begin(), shots.end());
    });
}

} // namespace server
