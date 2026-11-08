/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BossPart
*/

#ifndef SERVER_COMPONENTS_BOSSPART_HPP
#define SERVER_COMPONENTS_BOSSPART_HPP

#include "engine/Entity.hpp"
#include "engine/Vec2.hpp"

namespace server {

/**
 * @struct BossPart
 * @brief A part of a boss other than the entity holding its health: the
 * ceiling half of the twin, the head or the tail of the behemoth. It follows
 * its boss (bossPartSystem()), and the shots that hit it damage its boss
 * instead (damageSystem()). It is destroyed with its boss.
 * @param owner The boss entity.
 * @param offset Its position relative to the boss, in playfield units.
 * @param damageFactor How many times the damage of a shot the boss takes:
 * more than 1 for a weak point, less for an armored part.
 * @param swing How far it waves up and down around its offset, in playfield
 * units; 0 for a part that does not wave.
 * @param phase The angle of its wave, in radians.
 **/
struct BossPart {
    engine::Entity owner{};
    engine::Vec2f offset{};
    float damageFactor = 1.0F;
    float swing = 0.0F;
    float phase = 0.0F;
};

} // namespace server

#endif // SERVER_COMPONENTS_BOSSPART_HPP
