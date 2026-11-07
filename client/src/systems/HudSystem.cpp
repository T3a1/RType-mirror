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
#include "common/components/PowerUpKind.hpp"
#include "common/components/Transform.hpp"
#include "components/ShipOwner.hpp"
#include "engine/Entity.hpp"

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <string>

namespace client {

namespace {

/** @brief The font size of the label above a ship, in playfield units. */
constexpr int LabelFontSize = 28;
/** @brief How far above the ship's center its label is, in playfield units. */
constexpr float LabelOffsetY = 24.0F;
/** @brief The average glyph width of the default fonts, as a share of the font
 * size. */
constexpr float GlyphWidth = 0.55F;

/** @brief The left margin of the HUD and the right margin of the notices, in
 * pixels. */
constexpr float HudX = 20.0F;
/** @brief The top margin of the HUD, in pixels. */
constexpr float HudY = 20.0F;
/** @brief The font size of the HUD, in pixels. */
constexpr int HudFontSize = 28;
/** @brief The height of a line of the HUD, in pixels. */
constexpr float HudLineHeight = 34.0F;

/** @brief The font size of the chat, in pixels: a full line with its author
 * fits in the smallest window. */
constexpr int ChatFontSize = 22;
/** @brief The height of a line of the chat, in pixels. */
constexpr float ChatLineHeight = 28.0F;
/** @brief The distance between the chat and the bottom, in pixels. */
constexpr float ChatBottom = 20.0F;
/** @brief The space on the left and right of a text on its backdrop, in pixels.
 */
constexpr float ChatPadding = 6.0F;
/** @brief The backdrop of the chat, so that it stays readable over the game. */
constexpr gfx::Color ChatBackdrop{.r = 0, .g = 0, .b = 0, .a = 170};
/** @brief The color of the hints of the chat. */
constexpr gfx::Color ChatHintColor = gfx::rgb(170, 170, 185);

/** @brief The width of the game over panel, in pixels. */
constexpr float PanelWidth = 520.0F;
/** @brief The inner margin of the game over panel, in pixels. */
constexpr float PanelPadding = 24.0F;
/** @brief The font size of the titles, in pixels. */
constexpr int TitleFontSize = 56;
/** @brief The height of a title, in pixels. */
constexpr float TitleHeight = 72.0F;
/** @brief The width of the game over button, in pixels. */
constexpr float ButtonWidth = 300.0F;
/** @brief The height of the game over button, in pixels. */
constexpr float ButtonHeight = 52.0F;
/** @brief The background of the game over panel. */
constexpr gfx::Color PanelColor = gfx::rgb(24, 24, 36);

/** @brief How long a banner or a notice takes to fade out, in seconds. */
constexpr float BannerFade = 0.5F;

/** @brief The width of the boss's health bar, as a share of the window's width.
 */
constexpr float BossBarShare = 0.6F;
/** @brief The height of the boss's health bar, in pixels. */
constexpr float BossBarHeight = 16.0F;
/** @brief The distance between the boss's health bar and the bottom, in pixels.
 */
constexpr float BossBarBottom = 40.0F;
/** @brief The color of the boss's health bar. */
constexpr gfx::Color BossBarColor = gfx::rgb(213, 94, 0);
/** @brief The color of the level banner. */
constexpr gfx::Color BannerColor = gfx::rgb(255, 214, 90);

/**
 * @brief Fades a color out during the last BannerFade seconds.
 * @param color The color when fully visible.
 * @param timeLeft How long it is still shown, in seconds.
 * @return The color with its alpha reduced.
 **/
gfx::Color faded(gfx::Color color, float timeLeft)
{
    const float alpha = std::min(timeLeft / BannerFade, 1.0F);
    color.a = static_cast<std::uint8_t>(static_cast<float>(color.a) * alpha);
    return color;
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

void powerUpLabelSystem(engine::Registry &registry, gfx::IGraphics &graphics)
{
    registry.each<rtype::PowerUpKind, rtype::Transform>(
        [&graphics](engine::Entity, const rtype::PowerUpKind &kind,
                    const rtype::Transform &transform) {
            const std::string text{rtype::powerUpName(kind).front()};
            const float width = static_cast<float>(LabelFontSize) * GlyphWidth;
            graphics.drawText(text,
                              {.x = transform.position.x - (width / 2.0F),
                               .y = transform.position.y - LabelOffsetY -
                                    static_cast<float>(LabelFontSize)},
                              LabelFontSize, powerUpColor(kind));
        });
}

void hudSystem(const SyncState &state, bool autoFire, gfx::IGraphics &graphics)
{
    float y = HudY;
    const rtype::protocol::PlayerInfo *you = nullptr;
    for (const auto &player : state.players) {
        const bool local = player.slot == state.localSlot;
        const std::string text = "P" + std::to_string(player.slot) + "  " +
                                 std::to_string(player.score) +
                                 (local ? "  (you)" : "") +
                                 (player.shipId == 0 ? "  out" : "");
        graphics.drawText(text, {.x = HudX, .y = y}, HudFontSize,
                          playerColor(player.slot));
        y += HudLineHeight;
        if (local) {
            you = &player;
        }
    }
    for (const rtype::PowerUpKind kind : rtype::PowerUpKinds) {
        const std::uint8_t seconds =
            you != nullptr ? you->boosts.at(static_cast<std::size_t>(kind)) : 0;
        if (seconds == 0) {
            continue;
        }
        graphics.drawText(std::string{rtype::powerUpName(kind)} + "  " +
                              std::to_string(seconds) + "s",
                          {.x = HudX, .y = y}, HudFontSize, powerUpColor(kind));
        y += HudLineHeight;
    }
    if (autoFire) {
        graphics.drawText("Auto-fire ON", {.x = HudX, .y = y}, HudFontSize,
                          gfx::colors::OffWhite);
    }
}

void bossHealthBar(engine::Registry &registry, gfx::IGraphics &graphics)
{
    registry.each<rtype::EntityType, rtype::Health>(
        [&graphics](engine::Entity, const rtype::EntityType &type,
                    const rtype::Health &health) {
            if (!rtype::isBossEntity(type) || health.max <= 0) {
                return;
            }
            const engine::Vec2f screen = graphics.screenSize();
            const float ratio = std::clamp(static_cast<float>(health.current) /
                                               static_cast<float>(health.max),
                                           0.0F, 1.0F);
            const float width = screen.x * BossBarShare;
            gfx::Rect bar{.x = (screen.x - width) / 2.0F,
                          .y = screen.y - BossBarBottom,
                          .width = width,
                          .height = BossBarHeight};
            drawCentered(graphics,
                         type == rtype::EntityType::BossHive ? "HIVE"
                                                             : "WARSHIP",
                         screen.x / 2.0F, bar.y - HudLineHeight, HudFontSize,
                         gfx::colors::OffWhite);
            graphics.drawRectangle(bar, gfx::colors::DarkGray);
            bar.width = width * ratio;
            graphics.drawRectangle(bar, BossBarColor);
        });
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
    float y = (screen.y - TitleHeight) / 2.0F;
    if (state.level != 1) {
        drawCentered(graphics, "Level cleared", centerX, y - HudLineHeight,
                     HudFontSize,
                     faded(gfx::colors::OffWhite, state.levelBanner));
    }
    y += HudLineHeight / 2.0F;
    drawCentered(graphics,
                 state.level == 0 ? "ENDLESS MODE"
                                  : "LEVEL " + std::to_string(state.level),
                 centerX, y, TitleFontSize,
                 faded(BannerColor, state.levelBanner));
}

namespace {

/**
 * @brief Draws a line of the chat over its backdrop.
 * @param graphics The graphics backend to draw with.
 * @param text The text.
 * @param y The top of the line.
 * @param color The color of the text.
 **/
void drawChatRow(gfx::IGraphics &graphics, const std::string &text, float y,
                 gfx::Color color)
{
    const float width = static_cast<float>(text.size()) *
                        static_cast<float>(ChatFontSize) * GlyphWidth;
    gfx::Color backdrop = ChatBackdrop;
    backdrop.a = std::min(backdrop.a, color.a);
    graphics.drawRectangle({.x = HudX - ChatPadding,
                            .y = y,
                            .width = width + (2.0F * ChatPadding),
                            .height = ChatLineHeight},
                           backdrop);
    graphics.drawText(text, {.x = HudX, .y = y + 3.0F}, ChatFontSize, color);
}

} // namespace

void chatOverlay(const ChatState &chat, gfx::IGraphics &graphics)
{
    float y = graphics.screenSize().y - ChatBottom - ChatLineHeight;
    if (chat.open) {
        drawChatRow(graphics, "> " + chat.draft + "_", y,
                    gfx::colors::OffWhite);
        y -= ChatLineHeight;
        drawChatRow(graphics, "Enter: send   Esc: cancel", y, ChatHintColor);
        y -= ChatLineHeight;
    }
    for (const ChatLine &line : chat.lines | std::views::reverse) {
        drawChatRow(graphics,
                    "Player " + std::to_string(line.slot) + ": " + line.text, y,
                    faded(playerColor(line.slot), line.timeLeft));
        y -= ChatLineHeight;
    }
}

void chatHint(const std::string &keyName, gfx::IGraphics &graphics)
{
    drawChatRow(graphics, keyName + ": write to the room",
                graphics.screenSize().y - ChatBottom - ChatLineHeight,
                ChatHintColor);
}

void playerNoticeSystem(const SyncState &state, gfx::IGraphics &graphics)
{
    const float right = graphics.screenSize().x - HudX;
    float y = HudY;
    for (const PlayerNotice &notice : state.notices) {
        const std::string text = "Player " + std::to_string(notice.slot) +
                                 (notice.joined ? " joined" : " left");
        const float width = static_cast<float>(text.size()) *
                            static_cast<float>(HudFontSize) * GlyphWidth;
        graphics.drawText(text, {.x = right - width, .y = y}, HudFontSize,
                          faded(playerColor(notice.slot), notice.timeLeft));
        y += HudLineHeight;
    }
}

bool gameOverScreen(const SyncState &state, const MenuContext &context)
{
    MenuCursor cursor;
    MenuPanel panel(context, cursor);
    gfx::IGraphics &graphics = *context.graphics;
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
    return panel.backButton(button, "Back to lobby");
}

} // namespace client
