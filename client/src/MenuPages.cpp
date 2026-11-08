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

/** @brief The text of each control in the menus, in the order of Control. */
constexpr std::array<const char *, ControlCount> ControlLabels{
    "Up", "Down", "Left", "Right", "Fire", "Auto-fire", "Chat",
};
/** @brief How many keys of each control the Keyboard and Gamepad tabs show:
 * one per hand. */
constexpr std::size_t KeysPerRow = 2;
/** @brief The index of the Audio tab. */
constexpr std::size_t AudioTab = 0;
/** @brief The index of the Keyboard tab. */
constexpr std::size_t KeyboardTab = 1;
/** @brief The index of the Gamepad tab. */
constexpr std::size_t GamepadTab = 2;
/** @brief The height a row takes in a column, in pixels. */
constexpr float RowStep = MenuRowHeight + MenuRowGap;

/**
 * @brief Binds the key the player just pressed, if any, and reports if another
 * control uses it too.
 * @param context What the menu needs for this frame.
 * @param state The state of the Settings page, updated.
 * @param players The keys bound to each control on each device, updated.
 * @return Whether the keys changed.
 **/
bool applyCapture(const MenuContext &context, SettingsPageState &state,
                  PlayerControls &players)
{
    const std::optional<gfx::Key> key = state.capture.update(*context.graphics);
    const std::optional<KeySlot> slot = state.capture.slot();
    if (!key || !slot) {
        return false;
    }
    Controls &controls = state.capture.device() == Device::Gamepad
                             ? players.gamepad
                             : players.keyboard;
    bindKey(controls, slot->control, slot->index, *key);
    state.capture.cancel();
    context.sounds->play(audio::SoundEvent::MenuConfirm);
    const std::optional<Control> other =
        otherControlWith(controls, slot->control, *key);
    state.notice.clear();
    if (other) {
        state.notice = keyName(*key) + " is also used by " +
                       ControlLabels.at(static_cast<std::size_t>(*other));
    }
    return true;
}

/**
 * @brief Returns the text of the button of one key of a control.
 * @param state The state of the Settings page.
 * @param controls The keys bound to each control on the device of the tab.
 * @param device The device of the tab.
 * @param slot The key.
 * @return The control and its key, or a request while the key is changing.
 **/
std::string keyLabel(const SettingsPageState &state, const Controls &controls,
                     Device device, KeySlot slot)
{
    const auto control = static_cast<std::size_t>(slot.control);
    const std::string name = std::string{ControlLabels.at(control)} + ": ";
    const std::optional<KeySlot> waiting = state.capture.slot();
    if (waiting && state.capture.device() == device &&
        waiting->control == slot.control && waiting->index == slot.index) {
        return name + (device == Device::Gamepad ? "press a button..."
                                                 : "press a key...");
    }
    const std::vector<gfx::Key> &keys = controls.at(control);
    return name +
           (slot.index < keys.size() ? keyName(keys.at(slot.index)) : "-");
}

/**
 * @brief Draws the Audio tab.
 * @param panel The panel of the page.
 * @param screen The size of the window.
 * @param top The top of the tab.
 * @param settings The settings, updated.
 * @param result What the player did, updated.
 * @return The height below the tab.
 **/
float audioTab(MenuPanel &panel, engine::Vec2f screen, float top,
               Settings &settings, SettingsResult &result)
{
    float y = top;
    result.changed =
        panel.slider(menuRow(screen, y), "Music volume", settings.musicVolume);
    y += RowStep;
    result.changed =
        panel.slider(menuRow(screen, y), "Sound volume", settings.soundVolume,
                     audio::SoundEvent::PlayerShot) ||
        result.changed;
    return y + RowStep;
}

/**
 * @brief Draws the Keyboard or the Gamepad tab: two keys per control, then a
 * button that brings back the default keys of the device.
 * @param panel The panel of the page.
 * @param context What the menu needs for this frame.
 * @param top The top of the tab.
 * @param state The state of the Settings page, updated.
 * @param controls The keys bound to each control on the device, updated.
 * @param device The device of the tab.
 * @param result What the player did, updated.
 * @return The height below the tab.
 **/
float controlsTab(MenuPanel &panel, const MenuContext &context, float top,
                  SettingsPageState &state, Controls &controls, Device device,
                  SettingsResult &result)
{
    gfx::IGraphics &graphics = *context.graphics;
    const engine::Vec2f screen = graphics.screenSize();
    for (std::size_t i = 0; i < ControlCount * KeysPerRow; ++i) {
        const KeySlot slot{.control = static_cast<Control>(i / KeysPerRow),
                           .index = i % KeysPerRow};
        if (panel.button(menuGridCell(screen, top, i),
                         keyLabel(state, controls, device, slot))) {
            state.capture.start(slot, device);
            state.notice.clear();
        }
    }
    float y = top + (static_cast<float>(ControlCount) * RowStep);
    drawCentered(graphics, state.notice, screen.x / 2.0F, y, MenuTextFontSize,
                 gfx::colors::OffWhite);
    y += MenuLineHeight;
    if (panel.button(menuRow(screen, y), "Reset keys")) {
        controls = device == Device::Gamepad ? defaultPadControls()
                                             : defaultControls();
        state.capture.cancel();
        state.notice.clear();
        result.controlsChanged = true;
    }
    return y + RowStep;
}

/**
 * @brief Returns the text of each window size.
 * @return The texts, in the order of WindowSizes.
 **/
std::array<std::string, WindowSizes.size()> windowSizeNames()
{
    std::array<std::string, WindowSizes.size()> names;
    for (std::size_t i = 0; i < names.size(); ++i) {
        names.at(i) = windowSizeName(WindowSizes.at(i));
    }
    return names;
}

/**
 * @brief Returns the text of each frame limit.
 * @return The texts, in the order of FrameLimits.
 **/
std::array<std::string, FrameLimits.size()> frameLimitNames()
{
    std::array<std::string, FrameLimits.size()> names;
    for (std::size_t i = 0; i < names.size(); ++i) {
        const int limit = FrameLimits.at(i);
        names.at(i) =
            limit == 0 ? "No limit" : std::to_string(limit) + " per second";
    }
    return names;
}

/**
 * @brief Draws the Video tab. A row that does not apply stays in its place,
 * disabled, so that the keyboard selection does not jump.
 * @param panel The panel of the page.
 * @param screen The size of the window.
 * @param top The top of the tab.
 * @param settings The settings, updated.
 * @param result What the player did, updated.
 * @return The height below the tab.
 **/
float videoTab(MenuPanel &panel, engine::Vec2f screen, float top,
               Settings &settings, SettingsResult &result)
{
    float y = top;
    bool changed =
        panel.checkbox(menuRow(screen, y), "Fullscreen", settings.fullscreen);
    y += RowStep;
    if (settings.fullscreen) {
        static_cast<void>(panel.button(
            menuRow(screen, y),
            "Window size: " +
                windowSizeName(WindowSizes.at(settings.windowSize)),
            false));
    } else {
        const auto names = windowSizeNames();
        changed = panel.selector(menuRow(screen, y), "Window size", names,
                                 settings.windowSize) ||
                  changed;
    }
    y += RowStep;
    changed =
        panel.checkbox(menuRow(screen, y), "V-Sync", settings.vsync) || changed;
    y += RowStep;
    if (settings.vsync) {
        static_cast<void>(
            panel.button(menuRow(screen, y), "Frame limit: V-Sync", false));
    } else {
        const auto names = frameLimitNames();
        changed = panel.selector(menuRow(screen, y), "Frame limit", names,
                                 settings.frameLimit) ||
                  changed;
    }
    result.changed = changed;
    result.videoChanged = changed;
    return y + RowStep;
}

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

SettingsResult settingsPage(const MenuContext &context,
                            SettingsPageState &state, Settings &settings,
                            PlayerControls &controls)
{
    SettingsResult result;
    // The widgets must not take the keys pressed for the capture.
    MenuContext frame = context;
    if (state.capture.slot()) {
        result.controlsChanged = applyCapture(context, state, controls);
        frame.keys = MenuKeys{};
    }
    MenuPanel panel(frame, state.cursor);
    gfx::IGraphics &graphics = *context.graphics;
    const engine::Vec2f screen = graphics.screenSize();

    float y = drawMenuTitle(graphics, "SETTINGS");
    const std::array<std::string, 4> tabs{"Audio", "Keyboard", "Gamepad",
                                          "Video"};
    panel.tabs(menuRow(screen, y), tabs, state.tab);
    y += RowStep;
    // A capture only goes on in the tab of its device.
    const bool keyboardCapture =
        state.tab == KeyboardTab && state.capture.device() == Device::Keyboard;
    const bool gamepadCapture =
        state.tab == GamepadTab && state.capture.device() == Device::Gamepad;
    if (!keyboardCapture && !gamepadCapture) {
        state.capture.cancel();
    }
    if (state.tab == AudioTab) {
        y = audioTab(panel, screen, y, settings, result);
    } else if (state.tab == KeyboardTab) {
        y = controlsTab(panel, frame, y, state, controls.keyboard,
                        Device::Keyboard, result);
    } else if (state.tab == GamepadTab) {
        y = controlsTab(panel, frame, y, state, controls.gamepad,
                        Device::Gamepad, result);
    } else {
        y = videoTab(panel, screen, y, settings, result);
    }
    result.back = panel.backButton(menuRow(screen, y), "Back");
    if (result.back) {
        state.capture.cancel();
    }
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

std::array<std::string, 6> howToPlayLines(const Controls &controls)
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
        "Write to your room: " + keysOf(controls, Control::Chat) +
            ", then Enter to send",
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
