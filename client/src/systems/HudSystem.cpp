/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** HudSystem
*/

#include "systems/HudSystem.hpp"
#include "PlayerColors.hpp"
#include "common/components/Transform.hpp"
#include "components/ShipOwner.hpp"
#include "engine/Entity.hpp"

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
constexpr float ButtonWidth = 240.0F;
constexpr float ButtonHeight = 52.0F;
constexpr int ButtonFontSize = 28;
constexpr gfx::Color PanelColor = gfx::rgb(24, 24, 36);
constexpr gfx::Color ButtonColor = gfx::rgb(70, 70, 90);
constexpr gfx::Color ButtonHoverColor = gfx::rgb(110, 110, 140);

/**
 * @brief Draws one line of text centered around a point.
 * @param graphics The graphics backend to draw with.
 * @param text The text.
 * @param centerX The horizontal center of the text.
 * @param y The top of the text.
 * @param fontSize The font size.
 * @param color The color of the text.
 **/
void drawCentered(gfx::IGraphics &graphics, const std::string &text,
                  float centerX, float y, int fontSize, gfx::Color color)
{
    const float width = static_cast<float>(text.size()) *
                        static_cast<float>(fontSize) * GlyphWidth;
    graphics.drawText(text, {.x = centerX - (width / 2.0F), .y = y}, fontSize,
                      color);
}

/**
 * @brief Checks if a point is inside a rectangle.
 * @param rect The rectangle.
 * @param point The point.
 * @return Whether the rectangle contains the point.
 **/
bool contains(const gfx::Rect &rect, engine::Vec2f point)
{
    return point.x >= rect.x && point.x < rect.x + rect.width &&
           point.y >= rect.y && point.y < rect.y + rect.height;
}

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

void hudSystem(const SyncState &state, gfx::IGraphics &graphics)
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
}

bool gameOverScreen(const SyncState &state, gfx::IGraphics &graphics)
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
    const bool hovered = contains(button, graphics.mousePosition());
    graphics.drawRectangle(button, hovered ? ButtonHoverColor : ButtonColor);
    drawCentered(
        graphics, "Quit (Esc)", centerX,
        y + ((ButtonHeight - static_cast<float>(ButtonFontSize)) / 2.0F),
        ButtonFontSize, gfx::colors::OffWhite);
    return hovered && graphics.isMousePressed();
}

} // namespace client
