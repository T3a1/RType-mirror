/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Menu
*/

#include "Menu.hpp"

namespace client {

namespace {

// Average glyph width of the default fonts, as a share of the font size.
constexpr float GlyphWidth = 0.55F;
constexpr int ButtonFontSize = 28;
constexpr gfx::Color ButtonColor = gfx::rgb(70, 70, 90);
constexpr gfx::Color ButtonActiveColor = gfx::rgb(110, 110, 140);
constexpr gfx::Color ButtonDisabledColor = gfx::rgb(40, 40, 50);
constexpr gfx::Color LabelDisabledColor = gfx::rgb(130, 130, 140);

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

/**
 * @brief Detects a key going from released to held.
 * @param held Whether the key is held now.
 * @param wasHeld Whether it was held at the last call, updated.
 * @return Whether the key was just pressed.
 **/
bool pressed(bool held, bool &wasHeld)
{
    const bool result = held && !wasHeld;
    wasHeld = held;
    return result;
}

} // namespace

MenuKeys MenuInput::update(const gfx::IGraphics &graphics)
{
    return {
        .up = pressed(graphics.isKeyDown(gfx::Key::Up), m_up),
        .down = pressed(graphics.isKeyDown(gfx::Key::Down), m_down),
        .confirm = pressed(graphics.isKeyDown(gfx::Key::Space), m_confirm),
    };
}

void drawCentered(gfx::IGraphics &graphics, const std::string &text,
                  float centerX, float y, int fontSize, gfx::Color color)
{
    const float width = static_cast<float>(text.size()) *
                        static_cast<float>(fontSize) * GlyphWidth;
    graphics.drawText(text, {.x = centerX - (width / 2.0F), .y = y}, fontSize,
                      color);
}

bool menuButton(gfx::IGraphics &graphics, const gfx::Rect &rect,
                const std::string &label, ButtonStyle style)
{
    const bool hovered =
        style.enabled && contains(rect, graphics.mousePosition());
    gfx::Color color = ButtonDisabledColor;
    if (style.enabled) {
        color = hovered || style.selected ? ButtonActiveColor : ButtonColor;
    }
    graphics.drawRectangle(rect, color);
    drawCentered(
        graphics, label, rect.x + (rect.width / 2.0F),
        rect.y + ((rect.height - static_cast<float>(ButtonFontSize)) / 2.0F),
        ButtonFontSize,
        style.enabled ? gfx::colors::OffWhite : LabelDisabledColor);
    return hovered && graphics.isMousePressed();
}

} // namespace client
