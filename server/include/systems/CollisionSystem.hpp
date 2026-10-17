/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** CollisionSystem
*/

#ifndef SERVER_SYSTEMS_COLLISIONSYSTEM_HPP
#define SERVER_SYSTEMS_COLLISIONSYSTEM_HPP

#include "common/components/Collider.hpp"
#include "common/components/Transform.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"
#include "engine/Vec2.hpp"

#include <cmath>
#include <cstddef>
#include <vector>
namespace server {

/**
 * Two entities whose colliders overlap during this tick.
 * @param a The first entity.
 * @param b The second entity.
 **/
struct Collision {
    engine::Entity a{};
    engine::Entity b{};
};

std::vector<Collision> collisionSystem(engine::Registry &registry);

} // namespace server

#endif // SERVER_SYSTEMS_COLLISIONSYSTEM_HPP
