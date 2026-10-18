/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** CollisionSystem
*/

#include "systems/CollisionSystem.hpp"

namespace server {

namespace {

/**
 * What the collision system needs to know about an entity.
 * @param entity The entity itself.
 * @param position The position of the entity.
 * @param collider The collider of the entity.
 **/
struct Body {
    engine::Entity entity{};
    engine::Vec2f position{};
    rtype::Collider collider{};
};

/**
 * Get every entity that can collide.
 * @param registry The registry to read.
 * @return One body per entity with a Collider and a Transform.
 **/
std::vector<Body> collect(engine::Registry &registry)
{
    std::vector<Body> bodies;

    registry.each<rtype::Collider, rtype::Transform>(
        [&](engine::Entity entity, const rtype::Collider &collider,
            const rtype::Transform &transform) {
            bodies.push_back({.entity = entity,
                              .position = transform.position,
                              .collider = collider});
        });
    return bodies;
}

/**
 * Whether the layers of two bodies let them touch: one of them must have
 * the other's layer in its mask.
 * @param a The first body.
 * @param b The second body.
 **/
bool canCollide(const Body &a, const Body &b)
{
    return (a.collider.layer & b.collider.mask) != 0 ||
           (b.collider.layer & a.collider.mask) != 0;
}

/**
 * Whether the boxes of two bodies overlap (AABB)
 * @param a The first body.
 * @param b The second body.
 **/
bool overlaps(const Body &a, const Body &b)
{
    const float dx = std::abs(a.position.x - b.position.x);
    const float dy = std::abs(a.position.y - b.position.y);

    return dx < (a.collider.size.x + b.collider.size.x) / 2.0F &&
           dy < (a.collider.size.y + b.collider.size.y) / 2.0F;
}

} // namespace

/**
 * Finds every pair of entities that touch during this tick.
 * @param registry The registry to read.
 * @return The colliding pairs, each pair once.
 **/
std::vector<Collision> collisionSystem(engine::Registry &registry)
{
    const std::vector<Body> bodies = collect(registry);
    std::vector<Collision> collisions;

    for (std::size_t i = 0; i < bodies.size(); i++) {
        for (std::size_t j = i + 1; j < bodies.size(); j++) {
            if (canCollide(bodies[i], bodies[j]) &&
                overlaps(bodies[i], bodies[j])) {
                BB_DEBUG("Collision between " << bodies[i].entity.id << " and "
                                              << bodies[j].entity.id);
                collisions.push_back(
                    {.a = bodies[i].entity, .b = bodies[j].entity});
            }
        }
    }
    return collisions;
}

} // namespace server
