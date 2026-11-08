/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BoundarySystem
*/

#include "systems/BoundarySystem.hpp"

namespace server {

namespace {

/** @brief How far out of the playfield an entity may go, in playfield units. */
constexpr float OFFSCREEN_MARGIN = 200.0F;

/** @brief Checks if a position is offscreen.
 * @param position The position to check.
 * @return true if the position is offscreen, false otherwise.
 */
bool isOffScreen(const engine::Vec2f &position)
{
    return position.x < -OFFSCREEN_MARGIN ||
           position.x > rtype::PlayfieldWidth + OFFSCREEN_MARGIN ||
           position.y < -OFFSCREEN_MARGIN ||
           position.y > rtype::PlayfieldHeight + OFFSCREEN_MARGIN;
}

} // namespace

void boundarySystem(engine::Registry &registry, Outbound &outbound)
{
    registry.each<DestroyOffScreen, rtype::Transform>(
        [&](engine::Entity entity, const DestroyOffScreen &,
            const rtype::Transform &transform) {
            if (isOffScreen(transform.position)) {
                BB_DEBUG("Destroyed off-screen at x=" << transform.position.x);
                despawn(registry, entity, outbound);
            }
        });

    registry.each<KeepInPlayfield, rtype::Transform, rtype::Collider>(
        [](engine::Entity, const KeepInPlayfield &, rtype::Transform &transform,
           const rtype::Collider &collider) {
            transform.position =
                rtype::keepInPlayfield(transform.position, collider.size);
        });
}

} // namespace server