/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BoundarySystem
*/

#include "systems/BoundarySystem.hpp"

namespace server {

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

/**
 * Applies the playfield edges: destroys the DestroyOffScreen entities that
 * left it, and clamps the KeepInPlayfield ones inside it.
 * @param registry The registry to update.
 **/
void boundarySystem(engine::Registry &registry)
{
    registry.each<DestroyOffScreen, rtype::Transform>(
        [&](engine::Entity entity, const DestroyOffScreen &,
            const rtype::Transform &transform) {
            if (isOffScreen(transform.position)) {
                BB_DEBUG("Destroyed off-screen at x=" << transform.position.x);
                registry.destroy(entity);
            }
        });

    registry.each<KeepInPlayfield, rtype::Transform, rtype::Collider>(
        [](engine::Entity, const KeepInPlayfield &, rtype::Transform &transform,
           const rtype::Collider &collider) {
            const float halfWidth = collider.size.x / 2.0F;
            const float halfHeight = collider.size.y / 2.0F;

            transform.position.x =
                std::clamp(transform.position.x, halfWidth,
                           rtype::PlayfieldWidth - halfWidth);
            transform.position.y =
                std::clamp(transform.position.y, halfHeight,
                           rtype::PlayfieldHeight - halfHeight);
            BB_DEBUG("Clamped to " << transform.position.x << ", "
                                   << transform.position.y);
        });
}

} // namespace server