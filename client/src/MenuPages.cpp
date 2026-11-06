/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** MenuPages
*/

#include "MenuPages.hpp"

namespace client {

namespace {

/**
 * @struct MainEntry
 * @brief One button of the main menu.
 * @param label Its text.
 * @param choice What it chooses.
 **/
struct MainEntry {
    const char *label;
    MainChoice choice;
};

/** @brief The buttons of the main menu, from top to bottom. */
constexpr std::array MainEntries{
    MainEntry{.label = "Play", .choice = MainChoice::Play},
    MainEntry{.label = "Settings", .choice = MainChoice::Settings},
    MainEntry{.label = "Accessibility", .choice = MainChoice::Accessibility},
    MainEntry{.label = "How to play", .choice = MainChoice::HowToPlay},
    MainEntry{.label = "Credits", .choice = MainChoice::Credits},
    MainEntry{.label = "Quit", .choice = MainChoice::Quit},
};

/** @brief The lines of the Credits page, from top to bottom. */
constexpr std::array CreditsLines{
    "Made by",
    "Leandre Fouret, Romane Cauvez, Thomas Peze, David Caron",
    "",
    "Made with",
    "Raylib, SFML, ncurses, Asio, nlohmann-json, GoogleTest",
    "",
    "DejaVu font. Sounds generated for this project.",
};

/**
 * @brief Returns the keys of a control, as text.
 * @param controls The keys bound to each control.
 * @param control The control.
 * @return The names of its keys, "no key" if it has none.
 **/
std::string keysOf(const Controls &controls, Control control)
{
    std::string text;
    for (const gfx::Key key : controls.at(static_cast<std::size_t>(control))) {
        if (!text.empty()) {
            text += ", ";
        }
        text += keyName(key);
    }
    return text.empty() ? "no key" : text;
}

/**
 * @brief Draws lines of text, each centered in the window.
 * @param graphics The graphics backend to draw with.
 * @param lines The lines, from top to bottom.
 * @param top The top of the first line.
 * @return The height below the last line.
 **/
template <typename Lines>
float drawLines(gfx::IGraphics &graphics, const Lines &lines, float top)
{
    float y = top;
    for (const auto &line : lines) {
        drawCentered(graphics, line, graphics.screenSize().x / 2.0F, y,
                     MenuTextFontSize, gfx::colors::OffWhite);
        y += MenuLineHeight;
    }
    return y;
}

} // namespace

MainChoice mainMenuPage(const MenuContext &context, MenuCursor &cursor,
                        bool connected)
{
    MenuPanel panel(context, cursor);
    gfx::IGraphics &graphics = *context.graphics;
    const engine::Vec2f screen = graphics.screenSize();

    float y = drawMenuTitle(graphics, "R-TYPE");
    drawCentered(graphics, "Up, Down and Enter, or the mouse.", screen.x / 2.0F,
                 y, MenuTextFontSize, gfx::colors::OffWhite);
    y += MenuLineHeight;

    MainChoice choice = MainChoice::None;
    for (const MainEntry &entry : MainEntries) {
        const bool enabled = entry.choice != MainChoice::Play || connected;
        if (panel.button(menuRow(screen, y), entry.label, enabled)) {
            choice = entry.choice;
        }
        y += MenuRowHeight + MenuRowGap;
    }
    return choice;
}

SettingsResult settingsPage(const MenuContext &context, MenuCursor &cursor,
                            Settings &settings)
{
    MenuPanel panel(context, cursor);
    gfx::IGraphics &graphics = *context.graphics;
    const engine::Vec2f screen = graphics.screenSize();

    float y = drawMenuTitle(graphics, "SETTINGS");
    SettingsResult result;
    result.changed =
        panel.slider(menuRow(screen, y), "Music volume", settings.musicVolume);
    y += MenuRowHeight + MenuRowGap;
    result.changed =
        panel.slider(menuRow(screen, y), "Sound volume", settings.soundVolume,
                     audio::SoundEvent::PlayerShot) ||
        result.changed;
    y += MenuRowHeight + MenuRowGap;
    result.back = panel.backButton(menuRow(screen, y), "Back");
    return result;
}

SettingsResult accessibilityPage(const MenuContext &context, MenuCursor &cursor,
                                 Settings &settings)
{
    MenuPanel panel(context, cursor);
    gfx::IGraphics &graphics = *context.graphics;
    const engine::Vec2f screen = graphics.screenSize();

    float y = drawMenuTitle(graphics, "ACCESSIBILITY");
    SettingsResult result;
    for (const ToggleDef &toggle : Toggles) {
        result.changed = panel.checkbox(menuRow(screen, y), toggle.label,
                                        settings.*toggle.field) ||
                         result.changed;
        y += MenuRowHeight + MenuRowGap;
    }
    result.back = panel.backButton(menuRow(screen, y), "Back");
    return result;
}

std::array<std::string, 5> howToPlayLines(const Controls &controls)
{
    return {
        "Shoot the enemies and survive, alone or as a team.",
        "Move: " + keysOf(controls, Control::Up) + " / " +
            keysOf(controls, Control::Down) + " / " +
            keysOf(controls, Control::Left) + " / " +
            keysOf(controls, Control::Right),
        "Shoot: " + keysOf(controls, Control::Fire),
        "Charged shot: hold Shoot for 1 second, then release",
        "Auto-fire on or off: " + keysOf(controls, Control::AutoFire),
    };
}

bool howToPlayPage(const MenuContext &context, MenuCursor &cursor,
                   const Controls &controls)
{
    MenuPanel panel(context, cursor);
    gfx::IGraphics &graphics = *context.graphics;

    const float top = drawMenuTitle(graphics, "HOW TO PLAY");
    const float y = drawLines(graphics, howToPlayLines(controls), top);
    return panel.backButton(menuRow(graphics.screenSize(), y + MenuLineHeight),
                            "Back");
}

bool creditsPage(const MenuContext &context, MenuCursor &cursor)
{
    MenuPanel panel(context, cursor);
    gfx::IGraphics &graphics = *context.graphics;

    const float top = drawMenuTitle(graphics, "CREDITS");
    const float y = drawLines(graphics, CreditsLines, top);
    return panel.backButton(menuRow(graphics.screenSize(), y + MenuLineHeight),
                            "Back");
}

} // namespace client
