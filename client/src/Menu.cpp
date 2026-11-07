/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Menu
*/

#include "Menu.hpp"

namespace client {

namespace {

/** @brief The average glyph width of the default fonts, as a share of the font
 * size. */
constexpr float GlyphWidth = 0.55F;
/** @brief The font size of a button, in pixels. */
constexpr int ButtonFontSize = 28;
/** @brief The width of one character of a button, in pixels. */
constexpr float ButtonGlyph = static_cast<float>(ButtonFontSize) * GlyphWidth;
/** @brief The color of a button. */
constexpr gfx::Color ButtonColor = gfx::rgb(70, 70, 90);
/** @brief The color of a selected or hovered button. */
constexpr gfx::Color ButtonActiveColor = gfx::rgb(110, 110, 140);
/** @brief The color of a disabled button. */
constexpr gfx::Color ButtonDisabledColor = gfx::rgb(40, 40, 50);
/** @brief The color of the label of a disabled button. */
constexpr gfx::Color LabelDisabledColor = gfx::rgb(130, 130, 140);
/** @brief The space above the title of a menu, in pixels. */
constexpr float TitleTop = 24.0F;
/** @brief The height of the title of a menu, in pixels. */
constexpr float TitleHeight = 64.0F;
/** @brief The font size of the title of a menu, in pixels. */
constexpr int TitleFontSize = 48;
/** @brief The space between two columns of a grid, in pixels. */
constexpr float ColumnGap = 12.0F;
/** @brief The space inside a row, on its left and right, in pixels. */
constexpr float RowPadding = 12.0F;
/** @brief Where the bar of a slider starts, as a share of its row. */
constexpr float SliderBarStart = 0.5F;
/** @brief The height of the drawn bar of a slider, in pixels. */
constexpr float SliderBarHeight = 12.0F;
/** @brief The steps of a slider: 20 steps of 5%. */
constexpr long SliderSteps = 20;
/** @brief The characters kept for the percentage of a slider. */
constexpr float SliderPercentChars = 5.0F;

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

/**
 * @brief Moves the keyboard selection, wrapping around.
 * @param selected The selected item, updated.
 * @param count The number of items, not zero.
 * @param keys The menu keys of this frame.
 **/
void moveSelection(std::size_t &selected, std::size_t count,
                   const MenuKeys &keys)
{
    if (keys.up) {
        selected = (selected + count - 1) % count;
    }
    if (keys.down) {
        selected = (selected + 1) % count;
    }
}

/**
 * @brief Returns the top of a line of button text centered in a row.
 * @param rect The row.
 * @return The top of the text.
 **/
float textTop(const gfx::Rect &rect)
{
    return rect.y + ((rect.height - static_cast<float>(ButtonFontSize)) / 2.0F);
}

/**
 * @brief Draws the marker of the keyboard selection.
 * @param graphics The graphics backend.
 * @param x The left of the marker.
 * @param y The top of the marker.
 **/
void drawMarker(gfx::IGraphics &graphics, float x, float y)
{
    graphics.drawText(">", {.x = x, .y = y}, ButtonFontSize,
                      gfx::colors::OffWhite);
}

/**
 * @brief Draws the background and the left-aligned label of a row.
 * @param graphics The graphics backend.
 * @param rect The row.
 * @param label Its text.
 * @param selected Whether the keyboard selection is on it.
 **/
void drawRow(gfx::IGraphics &graphics, const gfx::Rect &rect,
             const std::string &label, bool selected)
{
    const bool active = selected || contains(rect, graphics.mousePosition());
    graphics.drawRectangle(rect, active ? ButtonActiveColor : ButtonColor);
    const float x = rect.x + RowPadding;
    if (selected) {
        drawMarker(graphics, x, textTop(rect));
    }
    graphics.drawText(label,
                      {.x = x + (2.0F * ButtonGlyph), .y = textTop(rect)},
                      ButtonFontSize, gfx::colors::OffWhite);
}

/**
 * @brief Returns the part of a slider that a click can move it with.
 * @param rect The row of the slider.
 * @return The bar, as high as the row.
 **/
gfx::Rect sliderBar(const gfx::Rect &rect)
{
    const float left = rect.x + (rect.width * SliderBarStart);
    const float right =
        rect.x + rect.width - RowPadding - (SliderPercentChars * ButtonGlyph);
    return {.x = left,
            .y = rect.y,
            .width = std::max(right - left, 1.0F),
            .height = rect.height};
}

/**
 * @brief Returns the rectangle of one tab of a row.
 * @param rect The row.
 * @param index The index of the tab.
 * @param count The number of tabs.
 * @return The rectangle.
 **/
gfx::Rect tabAt(const gfx::Rect &rect, std::size_t index, std::size_t count)
{
    const float width = rect.width / static_cast<float>(count);
    return {.x = rect.x + (static_cast<float>(index) * width),
            .y = rect.y,
            .width = width,
            .height = rect.height};
}

/**
 * @brief Tells whether one of some keys is held.
 * @param graphics The graphics backend.
 * @param keys The keys.
 * @return `true` if one of them is held.
 **/
bool anyDown(const gfx::IGraphics &graphics,
             std::initializer_list<gfx::Key> keys)
{
    return std::ranges::any_of(
        keys, [&graphics](gfx::Key key) { return graphics.isKeyDown(key); });
}

} // namespace

MenuKeys MenuInput::update(const gfx::IGraphics &graphics)
{
    using gfx::Key;
    return {
        .up = pressed(anyDown(graphics, {Key::Up, Key::PadUp, Key::PadStickUp}),
                      m_up),
        .down = pressed(
            anyDown(graphics, {Key::Down, Key::PadDown, Key::PadStickDown}),
            m_down),
        .left = pressed(
            anyDown(graphics, {Key::Left, Key::PadLeft, Key::PadStickLeft}),
            m_left),
        .right = pressed(
            anyDown(graphics, {Key::Right, Key::PadRight, Key::PadStickRight}),
            m_right),
        .confirm = pressed(
            anyDown(graphics, {Key::Space, Key::Enter, Key::PadA}), m_confirm),
        .back = pressed(graphics.isKeyDown(Key::Escape), m_back),
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

float drawMenuTitle(gfx::IGraphics &graphics, const std::string &title)
{
    drawCentered(graphics, title, graphics.screenSize().x / 2.0F, TitleTop,
                 TitleFontSize, gfx::colors::OffWhite);
    return TitleTop + TitleHeight;
}

gfx::Rect menuGridCell(engine::Vec2f screen, float top, std::size_t index)
{
    const float left = (screen.x - (2.0F * MenuRowWidth) - ColumnGap) / 2.0F;
    const std::size_t column = index % 2;
    const std::size_t row = index / 2;
    return {
        .x = left + (static_cast<float>(column) * (MenuRowWidth + ColumnGap)),
        .y = top + (static_cast<float>(row) * (MenuRowHeight + MenuRowGap)),
        .width = MenuRowWidth,
        .height = MenuRowHeight,
    };
}

gfx::Rect menuRow(engine::Vec2f screen, float y)
{
    return {
        .x = (screen.x - MenuRowWidth) / 2.0F,
        .y = y,
        .width = MenuRowWidth,
        .height = MenuRowHeight,
    };
}

namespace {

/**
 * @struct ButtonStyle
 * @brief How a menu button looks.
 * @param selected Whether the keyboard selection is on it.
 * @param enabled Whether it can be chosen; a disabled button is dimmed.
 **/
struct ButtonStyle {
    bool selected = false;
    bool enabled = true;
};

/**
 * @brief Draws a button with a centered label, in screen pixels. A selected
 * button shows a marker, so that the selection does not rely on color alone.
 * @param graphics The graphics backend to draw with and read the mouse from.
 * @param rect Where the button is.
 * @param label Its text.
 * @param style How it looks.
 * @return Whether it was clicked, never when disabled.
 **/
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
    const float centerX = rect.x + (rect.width / 2.0F);
    drawCentered(graphics, label, centerX, textTop(rect), ButtonFontSize,
                 style.enabled ? gfx::colors::OffWhite : LabelDisabledColor);
    if (style.selected) {
        const float labelWidth = static_cast<float>(label.size()) * ButtonGlyph;
        drawMarker(graphics,
                   centerX - (labelWidth / 2.0F) - (2.0F * ButtonGlyph),
                   textTop(rect));
    }
    return hovered && graphics.isMousePressed();
}

} // namespace

MenuPanel::MenuPanel(const MenuContext &context, MenuCursor &cursor)
    : m_graphics(context.graphics), m_keys(context.keys),
      m_sounds(context.sounds), m_cursor(&cursor)
{
    if (m_cursor->count == 0) {
        return;
    }
    const std::size_t before = m_cursor->selected;
    moveSelection(m_cursor->selected, m_cursor->count, m_keys);
    if (m_cursor->selected != before) {
        m_sounds->play(audio::SoundEvent::MenuMove);
    }
}

MenuPanel::~MenuPanel()
{
    m_cursor->count = m_next;
    m_cursor->selected =
        m_next == 0 ? 0 : std::min(m_cursor->selected, m_next - 1);
}

bool MenuPanel::nextIsSelected() { return m_next++ == m_cursor->selected; }

bool MenuPanel::pressButton(const gfx::Rect &rect, const std::string &label,
                            bool enabled, audio::SoundEvent sound)
{
    const bool selected = nextIsSelected();
    const bool clicked = menuButton(*m_graphics, rect, label,
                                    {.selected = selected, .enabled = enabled});
    const bool chosen = clicked || (selected && enabled && m_keys.confirm);
    if (chosen) {
        m_sounds->play(sound);
    }
    return chosen;
}

bool MenuPanel::button(const gfx::Rect &rect, const std::string &label,
                       bool enabled)
{
    return pressButton(rect, label, enabled, audio::SoundEvent::MenuConfirm);
}

bool MenuPanel::backButton(const gfx::Rect &rect, const std::string &label)
{
    return pressButton(rect, label, true, audio::SoundEvent::MenuBack);
}

bool MenuPanel::checkbox(const gfx::Rect &rect, const std::string &label,
                         bool &value)
{
    const bool selected = nextIsSelected();
    const bool clicked = m_graphics->isMousePressed() &&
                         contains(rect, m_graphics->mousePosition());
    const bool changed = clicked || (selected && m_keys.confirm);
    if (changed) {
        value = !value;
        m_sounds->play(audio::SoundEvent::MenuConfirm);
    }
    drawRow(*m_graphics, rect, (value ? "[x] " : "[ ] ") + label, selected);
    return changed;
}

bool MenuPanel::slider(const gfx::Rect &rect, const std::string &label,
                       float &value, audio::SoundEvent feedback)
{
    const bool selected = nextIsSelected();
    const gfx::Rect bar = sliderBar(rect);
    const engine::Vec2f mouse = m_graphics->mousePosition();
    const long before = std::lround(value * static_cast<float>(SliderSteps));
    long step = before;
    if (selected && m_keys.left) {
        --step;
    }
    if (selected && m_keys.right) {
        ++step;
    }
    if (m_graphics->isMousePressed() && contains(bar, mouse)) {
        step = std::lround((mouse.x - bar.x) / bar.width *
                           static_cast<float>(SliderSteps));
    }
    step = std::clamp(step, 0L, SliderSteps);
    const bool changed = step != before;
    if (changed) {
        value = static_cast<float>(step) / static_cast<float>(SliderSteps);
        m_sounds->play(feedback);
    }

    drawRow(*m_graphics, rect, label, selected);
    const gfx::Rect track{.x = bar.x,
                          .y =
                              rect.y + ((rect.height - SliderBarHeight) / 2.0F),
                          .width = bar.width,
                          .height = SliderBarHeight};
    m_graphics->drawRectangle(track, ButtonDisabledColor);
    m_graphics->drawRectangle({.x = track.x,
                               .y = track.y,
                               .width = track.width * static_cast<float>(step) /
                                        static_cast<float>(SliderSteps),
                               .height = track.height},
                              gfx::colors::OffWhite);
    const std::string percent = std::to_string(step * 100 / SliderSteps) + "%";
    m_graphics->drawText(
        percent, {.x = bar.x + bar.width + ButtonGlyph, .y = textTop(rect)},
        ButtonFontSize, gfx::colors::OffWhite);
    return changed;
}

bool MenuPanel::selector(const gfx::Rect &rect, const std::string &label,
                         std::span<const std::string> options,
                         std::size_t &current)
{
    const bool selected = nextIsSelected();
    if (options.empty()) {
        return false;
    }
    const std::size_t count = options.size();
    const std::size_t before = std::min(current, count - 1);
    const bool clicked = m_graphics->isMousePressed() &&
                         contains(rect, m_graphics->mousePosition());
    current = before;
    if (selected && m_keys.left) {
        current = (current + count - 1) % count;
    }
    if (clicked || (selected && (m_keys.right || m_keys.confirm))) {
        current = (current + 1) % count;
    }
    const bool changed = current != before;
    if (changed) {
        m_sounds->play(audio::SoundEvent::MenuMove);
    }
    drawRow(*m_graphics, rect, label + ": < " + options[current] + " >",
            selected);
    return changed;
}

void MenuPanel::tabs(const gfx::Rect &rect, std::span<const std::string> labels,
                     std::size_t &current)
{
    const bool selected = nextIsSelected();
    if (labels.empty()) {
        return;
    }
    const std::size_t count = labels.size();
    const std::size_t before = std::min(current, count - 1);
    current = before;
    if (selected && m_keys.left) {
        current = (current + count - 1) % count;
    }
    if (selected && m_keys.right) {
        current = (current + 1) % count;
    }
    const engine::Vec2f mouse = m_graphics->mousePosition();
    for (std::size_t i = 0; i < count; ++i) {
        if (m_graphics->isMousePressed() &&
            contains(tabAt(rect, i, count), mouse)) {
            current = i;
        }
    }
    if (current != before) {
        m_sounds->play(audio::SoundEvent::MenuMove);
    }

    if (selected) {
        drawMarker(*m_graphics, rect.x - (2.0F * ButtonGlyph), textTop(rect));
    }
    for (std::size_t i = 0; i < count; ++i) {
        const gfx::Rect tab = tabAt(rect, i, count);
        const bool active = i == current;
        m_graphics->drawRectangle(tab, active || contains(tab, mouse)
                                           ? ButtonActiveColor
                                           : ButtonColor);
        const std::string text = active ? "[" + labels[i] + "]" : labels[i];
        drawCentered(*m_graphics, text, tab.x + (tab.width / 2.0F),
                     textTop(rect), ButtonFontSize, gfx::colors::OffWhite);
    }
}

} // namespace client
