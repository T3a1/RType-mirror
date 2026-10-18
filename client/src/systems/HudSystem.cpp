/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** HudSystem
*/

#include "systems/HudSystem.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/Transform.hpp"
#include "engine/Entity.hpp"

#include <raylib.h>

#include <algorithm>
#include <string>

namespace client {

namespace {

constexpr float BarWidth = 64.0F;
constexpr float BarHeight = 8.0F;
constexpr float BarOffsetY = 44.0F; // above the ship's center
constexpr float LowHealth = 0.3F;

constexpr int HudX = 20;
constexpr int HudY = 20;
constexpr int HudFontSize = 28;
constexpr int HudLineHeight = 34;

} // namespace

void healthBarSystem(engine::Registry &registry)
{
    registry.each<rtype::EntityType, rtype::Transform, rtype::Health>(
        [](engine::Entity, const rtype::EntityType &type,
           const rtype::Transform &transform, const rtype::Health &health) {
            if (type != rtype::EntityType::PlayerShip || health.max <= 0) {
                return;
            }
            const float ratio = std::clamp(static_cast<float>(health.current) /
                                               static_cast<float>(health.max),
                                           0.0F, 1.0F);
            const Vector2 corner{
                .x = transform.position.x - (BarWidth / 2.0F),
                .y = transform.position.y - BarOffsetY,
            };
            DrawRectangleV(corner, {.x = BarWidth, .y = BarHeight}, DARKGRAY);
            DrawRectangleV(corner, {.x = BarWidth * ratio, .y = BarHeight},
                           ratio > LowHealth ? GREEN : RED);
        });
}

void hudSystem(const SyncState &state)
{
    int y = HudY;
    for (const auto &player : state.players) {
        const bool local = player.slot == state.localSlot;
        const std::string text = "P" + std::to_string(player.slot) + "  " +
                                 std::to_string(player.score) +
                                 (local ? "  (you)" : "");
        DrawText(text.c_str(), HudX, y, HudFontSize,
                 local ? SKYBLUE : RAYWHITE);
        y += HudLineHeight;
    }
}

} // namespace client
