/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** HudSystem
*/

#include "systems/HudSystem.hpp"
#include "Menu.hpp"
#include "PlayerColors.hpp"
#include "common/components/Transform.hpp"
#include "components/ShipOwner.hpp"
#include "engine/Entity.hpp"

#include <algorithm>
#include <cstdint>
#include <string>

namespace client {

namespace {

constexpr int LabelFontSize = 28;
constexpr float LabelOffsetY = 24.0F; // above the ship's center
// Average glyph width of the default fonts, as a share of the font size.
constexpr float GlyphWidth = 0.55F;

constexpr float HudX = 20.0F;
constexpr float HudY = 20.0F;
constexpr int HudFontSize = 28;
constexpr float HudLineHeight = 34.0F;

constexpr float PanelWidth = 520.0F;
constexpr float PanelPadding = 24.0F;
constexpr int TitleFontSize = 56;
constexpr float TitleHeight = 72.0F;
constexpr float ButtonWidth = 300.0F;
constexpr float ButtonHeight = 52.0F;
constexpr gfx::Color PanelColor = gfx::rgb(24, 24, 36);

constexpr float BannerFade = 0.5F;
constexpr gfx::Color BannerColor = gfx::rgb(255, 214, 90);

} // namespace

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
                               .y = transform.position.y - LabelOffsetY -
                                    static_cast<float>(LabelFontSize)},
                              LabelFontSize, playerColor(owner.slot));
        });
}

void hudSystem(const SyncState &state, bool autoFire, gfx::IGraphics &graphics)
{
    float y = HudY;
    for (const auto &player : state.players) {
        const bool local = player.slot == state.localSlot;
        const std::string text = "P" + std::to_string(player.slot) + "  " +
                                 std::to_string(player.score) +
                                 (local ? "  (you)" : "") +
                                 (player.shipId == 0 ? "  out" : "");
        graphics.drawText(text, {.x = HudX, .y = y}, HudFontSize,
                          playerColor(player.slot));
        y += HudLineHeight;
    }
    if (autoFire) {
        graphics.drawText("Auto-fire ON", {.x = HudX, .y = y}, HudFontSize,
                          gfx::colors::OffWhite);
    }
}

void levelBannerScreen(const SyncState &state, gfx::IGraphics &graphics)
{
    if (!state.levelAnnounced) {
        return;
    }
    const engine::Vec2f screen = graphics.screenSize();
    const float centerX = screen.x / 2.0F;
    const std::string name = state.level == 0
                                 ? "Endless mode"
                                 : "Level " + std::to_string(state.level);
    drawCentered(graphics, name, centerX, HudY, HudFontSize,
                 gfx::colors::OffWhite);
    if (state.levelBanner <= 0.0F) {
        return;
    }
    // Fully visible, then fades out during its last BannerFade seconds.
    const float alpha = std::min(state.levelBanner / BannerFade, 1.0F);
    const auto fade = [alpha](gfx::Color color) {
        color.a =
            static_cast<std::uint8_t>(static_cast<float>(color.a) * alpha);
        return color;
    };
    float y = (screen.y - TitleHeight) / 2.0F;
    if (state.level != 1) {
        drawCentered(graphics, "Level cleared", centerX, y - HudLineHeight,
                     HudFontSize, fade(gfx::colors::OffWhite));
    }
    y += HudLineHeight / 2.0F;
    drawCentered(graphics,
                 state.level == 0 ? "ENDLESS MODE"
                                  : "LEVEL " + std::to_string(state.level),
                 centerX, y, TitleFontSize, fade(BannerColor));
}

bool gameOverScreen(const SyncState &state, gfx::IGraphics &graphics,
                    bool confirm)
{
    const engine::Vec2f screen = graphics.screenSize();
    const auto lines = static_cast<float>(state.players.size() + 1);
    const float height = (PanelPadding * 3.0F) + TitleHeight +
                         (lines * HudLineHeight) + ButtonHeight;
    const float centerX = screen.x / 2.0F;
    float y = (screen.y - height) / 2.0F;

    graphics.drawRectangle({.x = centerX - (PanelWidth / 2.0F),
                            .y = y,
                            .width = PanelWidth,
                            .height = height},
                           PanelColor);
    y += PanelPadding;
    drawCentered(graphics, "GAME OVER", centerX, y, TitleFontSize,
                 gfx::colors::OffWhite);
    y += TitleHeight;

    std::uint32_t total = 0;
    for (const auto &player : state.players) {
        const bool local = player.slot == state.localSlot;
        drawCentered(graphics,
                     "P" + std::to_string(player.slot) + "  " +
                         std::to_string(player.score) +
                         (local ? "  (you)" : ""),
                     centerX, y, HudFontSize, playerColor(player.slot));
        total += player.score;
        y += HudLineHeight;
    }
    drawCentered(graphics, "Team score  " + std::to_string(total), centerX, y,
                 HudFontSize, gfx::colors::OffWhite);
    y += HudLineHeight + PanelPadding;

    const gfx::Rect button{.x = centerX - (ButtonWidth / 2.0F),
                           .y = y,
                           .width = ButtonWidth,
                           .height = ButtonHeight};
    return menuButton(graphics, button, "Back to lobby", {.selected = true}) ||
           confirm;
}

} // namespace client
