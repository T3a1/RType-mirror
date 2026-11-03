/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** HudSystem
*/

#include "systems/HudSystem.hpp"
#include "PlayerColors.hpp"
#include "common/components/EntityType.hpp"
#include "common/components/Health.hpp"
#include "common/components/Transform.hpp"
#include "components/ShipOwner.hpp"
#include "engine/Entity.hpp"

#include <algorithm>
#include <string>

namespace client {

namespace {

constexpr float BarWidth = 64.0F;
constexpr float BarHeight = 8.0F;
constexpr float BarOffsetY = 44.0F; // above the ship's center
constexpr float LowHealth = 0.3F;

constexpr int LabelFontSize = 28;
constexpr float LabelGap = 6.0F; // between the label and the health bar
// Average glyph width of the default fonts, as a share of the font size.
constexpr float GlyphWidth = 0.55F;

constexpr float HudX = 20.0F;
constexpr float HudY = 20.0F;
constexpr int HudFontSize = 28;
constexpr float HudLineHeight = 34.0F;

} // namespace

void healthBarSystem(engine::Registry &registry, gfx::IGraphics &graphics)
{
    registry.each<rtype::EntityType, rtype::Transform, rtype::Health>(
        [&graphics](engine::Entity, const rtype::EntityType &type,
                    const rtype::Transform &transform,
                    const rtype::Health &health) {
            if (type != rtype::EntityType::PlayerShip || health.max <= 0) {
                return;
            }
            const float ratio = std::clamp(static_cast<float>(health.current) /
                                               static_cast<float>(health.max),
                                           0.0F, 1.0F);
            gfx::Rect bar{
                .x = transform.position.x - (BarWidth / 2.0F),
                .y = transform.position.y - BarOffsetY,
                .width = BarWidth,
                .height = BarHeight,
            };
            graphics.drawRectangle(bar, gfx::colors::DarkGray);
            bar.width = BarWidth * ratio;
            graphics.drawRectangle(bar, ratio > LowHealth ? gfx::colors::Green
                                                          : gfx::colors::Red);
        });
}

void playerLabelSystem(engine::Registry &registry, const SyncState &state,
                       gfx::IGraphics &graphics)
{
    registry.each<ShipOwner, rtype::Transform>(
        [&graphics, &state](engine::Entity, const ShipOwner &owner,
                            const rtype::Transform &transform) {
            const bool local = owner.slot == state.localSlot;
            const std::string text =
                "P" + std::to_string(owner.slot) + (local ? " (you)" : "");
            const float width = static_cast<float>(text.size()) *
                                static_cast<float>(LabelFontSize) * GlyphWidth;
            graphics.drawText(text,
                              {.x = transform.position.x - (width / 2.0F),
                               .y = transform.position.y - BarOffsetY -
                                    LabelGap -
                                    static_cast<float>(LabelFontSize)},
                              LabelFontSize, playerColor(owner.slot));
        });
}

void hudSystem(const SyncState &state, gfx::IGraphics &graphics)
{
    float y = HudY;
    for (const auto &player : state.players) {
        const bool local = player.slot == state.localSlot;
        const std::string text = "P" + std::to_string(player.slot) + "  " +
                                 std::to_string(player.score) +
                                 (local ? "  (you)" : "");
        graphics.drawText(text, {.x = HudX, .y = y}, HudFontSize,
                          playerColor(player.slot));
        y += HudLineHeight;
    }
}

} // namespace client
