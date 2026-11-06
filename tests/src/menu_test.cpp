/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** menu_test
*/

#include "FakeAudio.hpp"
#include "Menu.hpp"
#include "MenuPages.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace {

using client::Control;
using client::MainChoice;
using client::MenuCursor;
using client::MenuInput;
using client::MenuKeys;
using client::MenuPanel;
using client::audio::SoundBoard;
namespace gfx = client::gfx;

constexpr gfx::Rect Row{
    .x = 100.0F, .y = 100.0F, .width = 520.0F, .height = 40.0F};
constexpr const char *MoveSound = "assets/sounds/menu_move.wav";
constexpr const char *ConfirmSound = "assets/sounds/menu_confirm.wav";
constexpr const char *BackSound = "assets/sounds/menu_back.wav";
constexpr const char *ShotSound = "assets/sounds/player_shot.wav";

// Holds the keys and the mouse of a test, and records the text drawn.
class FakeGraphics final : public gfx::IGraphics {
  public:
    std::set<gfx::Key> held;
    engine::Vec2f mouse{.x = -1.0F, .y = -1.0F};
    bool mousePressed = false;
    std::vector<std::string> texts;

    [[nodiscard]] bool shouldClose() override { return false; }
    [[nodiscard]] engine::Vec2f screenSize() const override
    {
        return {.x = 1280.0F, .y = 720.0F};
    }
    void beginFrame(gfx::Color /*clear*/) override {}
    void endFrame() override {}
    void beginWorld(float /*zoom*/) override {}
    void endWorld() override {}
    [[nodiscard]] gfx::Texture
    loadTexture(const std::string & /*path*/,
                gfx::TextureFilter /*filter*/) override
    {
        return {};
    }
    void unloadTexture(gfx::Texture /*texture*/) override {}
    void drawRectangle(gfx::Rect /*rect*/, gfx::Color /*color*/) override {}
    void drawTexture(gfx::Texture /*texture*/, gfx::Rect /*source*/,
                     gfx::Rect /*dest*/, gfx::Color /*tint*/) override
    {
    }
    void drawText(const std::string &text, engine::Vec2f /*position*/,
                  int /*fontSize*/, gfx::Color /*color*/) override
    {
        texts.push_back(text);
    }
    [[nodiscard]] bool isKeyDown(gfx::Key key) const override
    {
        return held.contains(key);
    }
    [[nodiscard]] engine::Vec2f mousePosition() const override { return mouse; }
    [[nodiscard]] bool isMousePressed() const override { return mousePressed; }
};

// What one menu needs from one test.
struct MenuRig {
    FakeAudio audio;
    SoundBoard board{audio};
    FakeGraphics graphics;
    MenuCursor cursor;

    // The sounds played so far, by file.
    [[nodiscard]] std::vector<std::string> played() const
    {
        std::vector<std::string> files;
        files.reserve(audio.playedSounds.size());
        for (const std::uint32_t id : audio.playedSounds) {
            files.push_back(audio.paths.at(id - 1));
        }
        return files;
    }

    [[nodiscard]] client::MenuContext context(const MenuKeys &keys = {})
    {
        return {.graphics = &graphics, .sounds = &board, .keys = keys};
    }

    // One frame of a menu of a few buttons; returns which were chosen.
    std::vector<bool> buttons(std::size_t count, const MenuKeys &keys = {})
    {
        MenuPanel panel(context(keys), cursor);
        std::vector<bool> chosen;
        for (std::size_t i = 0; i < count; ++i) {
            chosen.push_back(panel.button(Row, "Item"));
        }
        return chosen;
    }

    void click(float x)
    {
        graphics.mouse = {.x = x, .y = Row.y + 1.0F};
        graphics.mousePressed = true;
    }

    [[nodiscard]] bool drew(const std::string &text) const
    {
        return std::find(graphics.texts.begin(), graphics.texts.end(), text) !=
               graphics.texts.end();
    }
};

// The Settings page and what it changes.
struct SettingsRig : MenuRig {
    client::SettingsPageState page;
    client::Settings settings;
    client::Controls controls = client::defaultControls();

    client::SettingsResult frame(const MenuKeys &keys = {})
    {
        return client::settingsPage(context(keys), page, settings, controls);
    }

    [[nodiscard]] const std::vector<gfx::Key> &keysOf(Control control) const
    {
        return controls.at(static_cast<std::size_t>(control));
    }
};

// The second key of Fire among the widgets: after the tabs, two per control.
constexpr std::size_t FireSecondKey =
    1 + (static_cast<std::size_t>(client::Control::Fire) * 2) + 1;

} // namespace

TEST(MenuInput, CountsAHeldKeyOnce)
{
    FakeGraphics graphics;
    MenuInput input;
    graphics.held.insert(gfx::Key::Left);

    EXPECT_TRUE(input.update(graphics).left);
    EXPECT_FALSE(input.update(graphics).left);

    graphics.held.clear();
    EXPECT_FALSE(input.update(graphics).left);
    graphics.held.insert(gfx::Key::Left);
    EXPECT_TRUE(input.update(graphics).left);
}

TEST(MenuInput, ReadsEveryKey)
{
    FakeGraphics graphics;
    MenuInput input;
    graphics.held = {gfx::Key::Up, gfx::Key::Down, gfx::Key::Left,
                     gfx::Key::Right, gfx::Key::Escape};

    const MenuKeys keys = input.update(graphics);

    EXPECT_TRUE(keys.up);
    EXPECT_TRUE(keys.down);
    EXPECT_TRUE(keys.left);
    EXPECT_TRUE(keys.right);
    EXPECT_TRUE(keys.back);
    EXPECT_FALSE(keys.confirm);
}

TEST(MenuInput, ConfirmsWithSpaceOrEnter)
{
    FakeGraphics graphics;
    MenuInput input;

    graphics.held = {gfx::Key::Space};
    EXPECT_TRUE(input.update(graphics).confirm);
    graphics.held.clear();
    EXPECT_FALSE(input.update(graphics).confirm);
    graphics.held = {gfx::Key::Enter};
    EXPECT_TRUE(input.update(graphics).confirm);

    // Going from one held key to the other is not a new press.
    graphics.held = {gfx::Key::Space};
    EXPECT_FALSE(input.update(graphics).confirm);
}

TEST(MenuPanel, DownMovesTheSelectionAndWraps)
{
    MenuRig rig;
    rig.buttons(3);

    rig.buttons(3, {.down = true});
    EXPECT_EQ(rig.cursor.selected, 1U);
    rig.buttons(3, {.down = true});
    rig.buttons(3, {.down = true});
    EXPECT_EQ(rig.cursor.selected, 0U);

    EXPECT_EQ(rig.played(),
              (std::vector<std::string>{MoveSound, MoveSound, MoveSound}));
}

TEST(MenuPanel, UpWrapsToTheLastWidget)
{
    MenuRig rig;
    rig.buttons(3);

    rig.buttons(3, {.up = true});

    EXPECT_EQ(rig.cursor.selected, 2U);
}

TEST(MenuPanel, DoesNotMoveBeforeItKnowsItsWidgets)
{
    MenuRig rig;

    rig.buttons(3, {.down = true});

    EXPECT_EQ(rig.cursor.selected, 0U);
    EXPECT_TRUE(rig.played().empty());
}

TEST(MenuPanel, StaysSilentWhenTheSelectionCannotMove)
{
    MenuRig rig;
    rig.buttons(1);

    rig.buttons(1, {.down = true});

    EXPECT_TRUE(rig.played().empty());
}

TEST(MenuPanel, KeepsTheSelectionOnAWidgetWhenTheMenuShrinks)
{
    MenuRig rig;
    rig.buttons(5);
    rig.buttons(5, {.up = true});
    ASSERT_EQ(rig.cursor.selected, 4U);

    rig.buttons(2);

    EXPECT_EQ(rig.cursor.selected, 1U);
    EXPECT_EQ(rig.cursor.count, 2U);
}

TEST(MenuPanel, ConfirmChoosesTheSelectedButtonOnly)
{
    MenuRig rig;
    rig.buttons(3);
    rig.buttons(3, {.down = true});

    const std::vector<bool> chosen = rig.buttons(3, {.confirm = true});

    EXPECT_EQ(chosen, (std::vector<bool>{false, true, false}));
    EXPECT_EQ(rig.played().back(), ConfirmSound);
}

TEST(MenuPanel, ClickChoosesAButtonWithoutTheSelection)
{
    MenuRig rig;
    rig.buttons(1);
    rig.cursor.selected = 5;
    rig.click(Row.x + 1.0F);

    MenuPanel panel(rig.context(), rig.cursor);

    EXPECT_TRUE(panel.button(Row, "Item"));
    EXPECT_EQ(rig.played(), std::vector<std::string>{ConfirmSound});
}

TEST(MenuPanel, ADisabledButtonCannotBeChosen)
{
    MenuRig rig;
    rig.click(Row.x + 1.0F);

    MenuPanel panel(rig.context({.confirm = true}), rig.cursor);

    EXPECT_FALSE(panel.button(Row, "Item", false));
    EXPECT_TRUE(rig.played().empty());
}

TEST(MenuPanel, ABackButtonPlaysTheBackSound)
{
    MenuRig rig;

    MenuPanel panel(rig.context({.confirm = true}), rig.cursor);

    EXPECT_TRUE(panel.backButton(Row, "Back"));
    EXPECT_EQ(rig.played(), std::vector<std::string>{BackSound});
}

TEST(MenuPanel, MarksTheSelectedWidgetWithText)
{
    MenuRig rig;
    rig.buttons(1);
    EXPECT_TRUE(rig.drew(">"));

    rig.graphics.texts.clear();
    rig.cursor.selected = 1;
    MenuPanel panel(rig.context(), rig.cursor);
    static_cast<void>(panel.button(Row, "First"));
    EXPECT_FALSE(rig.drew(">"));
}

TEST(MenuPanel, ConfirmTogglesACheckbox)
{
    MenuRig rig;
    bool value = false;

    MenuPanel panel(rig.context({.confirm = true}), rig.cursor);

    EXPECT_TRUE(panel.checkbox(Row, "Labels", value));
    EXPECT_TRUE(value);
    EXPECT_TRUE(rig.drew("[x] Labels"));
    EXPECT_EQ(rig.played(), std::vector<std::string>{ConfirmSound});
}

TEST(MenuPanel, ClickTogglesACheckboxOff)
{
    MenuRig rig;
    bool value = true;
    rig.click(Row.x + 1.0F);

    MenuPanel panel(rig.context(), rig.cursor);

    EXPECT_TRUE(panel.checkbox(Row, "Labels", value));
    EXPECT_FALSE(value);
    EXPECT_TRUE(rig.drew("[ ] Labels"));
}

TEST(MenuPanel, ACheckboxStaysAloneWithoutInput)
{
    MenuRig rig;
    bool value = true;

    MenuPanel panel(rig.context(), rig.cursor);

    EXPECT_FALSE(panel.checkbox(Row, "Labels", value));
    EXPECT_TRUE(value);
    EXPECT_TRUE(rig.played().empty());
}

TEST(MenuPanel, LeftAndRightMoveASliderBySteps)
{
    MenuRig rig;
    float value = 0.4F;
    {
        MenuPanel panel(rig.context({.right = true}), rig.cursor);
        EXPECT_TRUE(panel.slider(Row, "Music", value));
    }
    EXPECT_FLOAT_EQ(value, 0.45F);
    {
        MenuPanel panel(rig.context({.left = true}), rig.cursor);
        EXPECT_TRUE(panel.slider(Row, "Music", value));
    }
    EXPECT_FLOAT_EQ(value, 0.4F);
    EXPECT_EQ(rig.played(), (std::vector<std::string>{MoveSound, MoveSound}));
    EXPECT_TRUE(rig.drew("40%"));
}

TEST(MenuPanel, ASliderStopsAtItsBounds)
{
    MenuRig rig;
    float low = 0.0F;
    float high = 1.0F;
    {
        MenuPanel panel(rig.context({.left = true}), rig.cursor);
        EXPECT_FALSE(panel.slider(Row, "Low", low));
    }
    {
        MenuPanel panel(rig.context({.right = true}), rig.cursor);
        EXPECT_FALSE(panel.slider(Row, "High", high));
    }
    EXPECT_FLOAT_EQ(low, 0.0F);
    EXPECT_FLOAT_EQ(high, 1.0F);
    EXPECT_TRUE(rig.played().empty());
}

TEST(MenuPanel, OnlyTheSelectedSliderMoves)
{
    MenuRig rig;
    float first = 0.5F;
    float second = 0.5F;

    MenuPanel panel(rig.context({.right = true}), rig.cursor);
    panel.slider(Row, "First", first);
    panel.slider(Row, "Second", second);

    EXPECT_FLOAT_EQ(first, 0.55F);
    EXPECT_FLOAT_EQ(second, 0.5F);
}

TEST(MenuPanel, ClickOnTheBarSetsASlider)
{
    MenuRig rig;
    float value = 0.0F;
    // The bar starts in the middle of the row and leaves room for the percent.
    const float start = Row.x + (Row.width / 2.0F);
    rig.click(start);

    MenuPanel panel(rig.context(), rig.cursor);

    EXPECT_FALSE(panel.slider(Row, "Music", value));
    rig.click(start + 100.0F);
    EXPECT_TRUE(panel.slider(Row, "Music", value));
    EXPECT_GT(value, 0.0F);
    EXPECT_LE(value, 1.0F);
}

TEST(MenuPanel, ClickOnTheLabelDoesNotMoveASlider)
{
    MenuRig rig;
    float value = 0.5F;
    rig.click(Row.x + 1.0F);

    MenuPanel panel(rig.context(), rig.cursor);

    EXPECT_FALSE(panel.slider(Row, "Music", value));
    EXPECT_FLOAT_EQ(value, 0.5F);
}

TEST(MenuPanel, LeftAndRightChangeTheTabAndWrap)
{
    MenuRig rig;
    const std::array<std::string, 2> labels{"Audio", "Controls"};
    std::size_t current = 0;
    {
        MenuPanel panel(rig.context({.left = true}), rig.cursor);
        panel.tabs(Row, labels, current);
    }
    EXPECT_EQ(current, 1U);
    EXPECT_TRUE(rig.drew("[Controls]"));
    {
        MenuPanel panel(rig.context({.right = true}), rig.cursor);
        panel.tabs(Row, labels, current);
    }
    EXPECT_EQ(current, 0U);
    EXPECT_EQ(rig.played(), (std::vector<std::string>{MoveSound, MoveSound}));
}

TEST(MenuPanel, TabsOnlyListenToTheKeysWhenSelected)
{
    MenuRig rig;
    const std::array<std::string, 2> labels{"Audio", "Controls"};
    std::size_t current = 0;
    rig.cursor.selected = 1;
    rig.cursor.count = 2;

    MenuPanel panel(rig.context({.right = true}), rig.cursor);
    panel.tabs(Row, labels, current);

    EXPECT_EQ(current, 0U);
}

TEST(MenuPanel, ClickChoosesATab)
{
    MenuRig rig;
    const std::array<std::string, 2> labels{"Audio", "Controls"};
    std::size_t current = 0;
    rig.click(Row.x + Row.width - 1.0F);

    MenuPanel panel(rig.context(), rig.cursor);
    panel.tabs(Row, labels, current);

    EXPECT_EQ(current, 1U);
    EXPECT_EQ(rig.played(), std::vector<std::string>{MoveSound});
}

TEST(MenuPanel, EmptyTabsStillCountAsAWidget)
{
    MenuRig rig;
    std::size_t current = 3;
    {
        MenuPanel panel(rig.context(), rig.cursor);
        panel.tabs(Row, {}, current);
    }
    EXPECT_EQ(rig.cursor.count, 1U);
    EXPECT_EQ(current, 3U);
}

TEST(MenuPages, MainMenuOffersItsChoicesInOrder)
{
    MenuRig rig;
    static_cast<void>(client::mainMenuPage(rig.context(), rig.cursor, true));

    const MenuKeys confirm{.confirm = true};
    EXPECT_EQ(client::mainMenuPage(rig.context(confirm), rig.cursor, true),
              MainChoice::Play);
    const std::array<MainChoice, 5> others{
        MainChoice::Settings, MainChoice::Accessibility, MainChoice::HowToPlay,
        MainChoice::Credits, MainChoice::Quit};
    for (const MainChoice expected : others) {
        static_cast<void>(client::mainMenuPage(rig.context({.down = true}),
                                               rig.cursor, true));
        EXPECT_EQ(client::mainMenuPage(rig.context(confirm), rig.cursor, true),
                  expected);
    }
}

TEST(MenuPages, MainMenuChoosesNothingWithoutInput)
{
    MenuRig rig;

    EXPECT_EQ(client::mainMenuPage(rig.context(), rig.cursor, true),
              MainChoice::None);
}

TEST(MenuPages, PlayNeedsAConnection)
{
    MenuRig rig;

    EXPECT_EQ(
        client::mainMenuPage(rig.context({.confirm = true}), rig.cursor, false),
        MainChoice::None);
    EXPECT_TRUE(rig.played().empty());
}

TEST(MenuPages, TheOtherEntriesWorkWithoutAConnection)
{
    MenuRig rig;
    static_cast<void>(client::mainMenuPage(rig.context(), rig.cursor, false));
    static_cast<void>(
        client::mainMenuPage(rig.context({.up = true}), rig.cursor, false));

    EXPECT_EQ(
        client::mainMenuPage(rig.context({.confirm = true}), rig.cursor, false),
        MainChoice::Quit);
}

TEST(MenuPages, HowToPlayShowsTheDefaultKeys)
{
    const auto lines = client::howToPlayLines(client::defaultControls());

    EXPECT_EQ(lines.at(1), "Move: Up, W / Down, S / Left, A / Right, D");
    EXPECT_EQ(lines.at(2), "Shoot: Space, RightCtrl");
    EXPECT_EQ(lines.at(4), "Auto-fire on or off: F, RightShift");
}

TEST(MenuPages, HowToPlayFollowsTheControls)
{
    client::Controls controls = client::defaultControls();
    controls.at(static_cast<std::size_t>(client::Control::Fire)) = {
        gfx::Key::Enter};
    controls.at(static_cast<std::size_t>(client::Control::AutoFire)).clear();

    const auto lines = client::howToPlayLines(controls);

    EXPECT_EQ(lines.at(2), "Shoot: Enter");
    EXPECT_EQ(lines.at(4), "Auto-fire on or off: no key");
}

TEST(MenuPages, HowToPlayDrawsItsLines)
{
    MenuRig rig;
    const client::Controls controls = client::defaultControls();

    static_cast<void>(
        client::howToPlayPage(rig.context(), rig.cursor, controls));

    EXPECT_TRUE(rig.drew(client::howToPlayLines(controls).at(2)));
}

TEST(MenuPages, HowToPlayGoesBack)
{
    MenuRig rig;
    const client::Controls controls = client::defaultControls();

    EXPECT_FALSE(client::howToPlayPage(rig.context(), rig.cursor, controls));
    EXPECT_TRUE(client::howToPlayPage(rig.context({.confirm = true}),
                                      rig.cursor, controls));
    EXPECT_EQ(rig.played(), std::vector<std::string>{BackSound});
}

TEST(MenuPages, CreditsNameTheTeamAndGoBack)
{
    MenuRig rig;

    EXPECT_FALSE(client::creditsPage(rig.context(), rig.cursor));
    EXPECT_TRUE(
        rig.drew("Leandre Fouret, Romane Cauvez, Thomas Peze, David Caron"));
    EXPECT_TRUE(
        client::creditsPage(rig.context({.confirm = true}), rig.cursor));
    EXPECT_EQ(rig.played(), std::vector<std::string>{BackSound});
}

TEST(MenuPanel, ASliderPlaysTheSoundItIsGiven)
{
    MenuRig rig;
    float value = 0.5F;

    MenuPanel panel(rig.context({.right = true}), rig.cursor);
    panel.slider(Row, "Sound", value, client::audio::SoundEvent::PlayerShot);

    EXPECT_EQ(rig.played(), std::vector<std::string>{ShotSound});
}

TEST(MenuPages, SettingsChangeTheMusicVolume)
{
    SettingsRig rig;
    rig.page.cursor.selected = 1;

    const client::SettingsResult result = rig.frame({.left = true});

    EXPECT_TRUE(result.changed);
    EXPECT_FALSE(result.back);
    EXPECT_FLOAT_EQ(rig.settings.musicVolume, 0.35F);
    EXPECT_EQ(rig.played(), std::vector<std::string>{MoveSound});
}

TEST(MenuPages, SettingsTestTheSoundVolume)
{
    SettingsRig rig;
    rig.page.cursor.selected = 2;

    const client::SettingsResult result = rig.frame({.left = true});

    EXPECT_TRUE(result.changed);
    EXPECT_FLOAT_EQ(rig.settings.soundVolume, 0.95F);
    EXPECT_EQ(rig.played(), std::vector<std::string>{ShotSound});
}

TEST(MenuPages, SettingsReportNothingWithoutInput)
{
    SettingsRig rig;

    const client::SettingsResult result = rig.frame();

    EXPECT_FALSE(result.changed);
    EXPECT_FALSE(result.controlsChanged);
    EXPECT_FALSE(result.back);
    EXPECT_TRUE(rig.played().empty());
}

TEST(MenuPages, SettingsGoBack)
{
    SettingsRig rig;
    rig.page.cursor.selected = 3;

    const client::SettingsResult result = rig.frame({.confirm = true});

    EXPECT_TRUE(result.back);
    EXPECT_FALSE(result.changed);
    EXPECT_EQ(rig.played(), std::vector<std::string>{BackSound});
}

TEST(MenuPages, SettingsSwitchToControlsWithTheTabs)
{
    SettingsRig rig;

    static_cast<void>(rig.frame({.right = true}));
    EXPECT_EQ(rig.page.tab, 1U);

    rig.graphics.texts.clear();
    static_cast<void>(rig.frame());
    EXPECT_TRUE(rig.drew("[Controls]"));
    EXPECT_TRUE(rig.drew("Fire: Space"));
    EXPECT_TRUE(rig.drew("Fire: RightCtrl"));
}

TEST(MenuPages, ControlsTabShowsTwoKeysPerControl)
{
    SettingsRig rig;
    rig.page.tab = 1;

    static_cast<void>(rig.frame());

    // The tabs, two keys per control, Reset keys and Back.
    EXPECT_EQ(rig.page.cursor.count, 1 + (client::ControlCount * 2) + 2);
}

TEST(MenuPages, ChoosingAKeyStartsItsCapture)
{
    SettingsRig rig;
    rig.page.tab = 1;
    rig.page.cursor.selected = FireSecondKey;

    static_cast<void>(rig.frame({.confirm = true}));

    const client::KeySlot slot = rig.page.capture.slot().value_or(
        client::KeySlot{.control = Control::Up, .index = 0});
    EXPECT_EQ(slot.control, Control::Fire);
    EXPECT_EQ(slot.index, 1U);

    rig.graphics.texts.clear();
    static_cast<void>(rig.frame());
    EXPECT_TRUE(rig.drew("Fire: press a key..."));
}

TEST(MenuPages, CaptureWaitsForTheKeyThatStartedIt)
{
    SettingsRig rig;
    rig.page.tab = 1;
    rig.page.cursor.selected = FireSecondKey;
    rig.graphics.held = {gfx::Key::Enter};
    static_cast<void>(rig.frame({.confirm = true}));

    // Enter is still held: it must not become the new key.
    EXPECT_FALSE(rig.frame().controlsChanged);
    EXPECT_TRUE(rig.page.capture.slot().has_value());

    rig.graphics.held.clear();
    EXPECT_FALSE(rig.frame().controlsChanged);

    rig.graphics.held = {gfx::Key::G};
    EXPECT_TRUE(rig.frame().controlsChanged);
    EXPECT_FALSE(rig.page.capture.slot().has_value());
    EXPECT_EQ(rig.keysOf(Control::Fire),
              (std::vector<gfx::Key>{gfx::Key::Space, gfx::Key::G}));
}

TEST(MenuPages, CaptureNeverTakesEscape)
{
    SettingsRig rig;
    rig.page.tab = 1;
    rig.page.capture.start({.control = Control::Fire, .index = 0});
    static_cast<void>(rig.frame());

    rig.graphics.held = {gfx::Key::Escape};

    EXPECT_FALSE(rig.frame().controlsChanged);
    EXPECT_TRUE(rig.page.capture.slot().has_value());
}

TEST(MenuPages, TheCapturedKeyDoesNotReachTheWidgets)
{
    SettingsRig rig;
    rig.page.tab = 1;
    rig.page.cursor.selected = FireSecondKey;
    rig.page.capture.start({.control = Control::Fire, .index = 1});
    static_cast<void>(rig.frame());

    rig.graphics.held = {gfx::Key::Enter};
    EXPECT_TRUE(rig.frame({.confirm = true}).controlsChanged);

    // Enter is the new key: it does not start another capture.
    EXPECT_FALSE(rig.page.capture.slot().has_value());
    EXPECT_EQ(rig.played(), std::vector<std::string>{ConfirmSound});
}

TEST(MenuPages, AKeyUsedElsewhereIsReported)
{
    SettingsRig rig;
    rig.page.tab = 1;
    rig.page.capture.start({.control = Control::Fire, .index = 1});
    static_cast<void>(rig.frame());

    rig.graphics.held = {gfx::Key::F};
    EXPECT_TRUE(rig.frame().controlsChanged);

    EXPECT_EQ(rig.page.notice, "F is also used by Auto-fire");
    EXPECT_EQ(rig.keysOf(Control::Fire),
              (std::vector<gfx::Key>{gfx::Key::Space, gfx::Key::F}));
}

TEST(MenuPages, ResetKeysBringsBackTheDefaults)
{
    SettingsRig rig;
    rig.page.tab = 1;
    rig.controls.at(static_cast<std::size_t>(Control::Fire)) = {gfx::Key::G};
    rig.page.notice = "old";
    rig.page.cursor.selected = 1 + (client::ControlCount * 2);

    EXPECT_TRUE(rig.frame({.confirm = true}).controlsChanged);

    EXPECT_EQ(rig.controls, client::defaultControls());
    EXPECT_TRUE(rig.page.notice.empty());
}

TEST(MenuPages, LeavingTheControlsTabCancelsTheCapture)
{
    SettingsRig rig;
    rig.page.tab = 1;
    rig.page.capture.start({.control = Control::Fire, .index = 0});
    rig.page.tab = 0;

    static_cast<void>(rig.frame());

    EXPECT_FALSE(rig.page.capture.slot().has_value());
}

TEST(MenuPages, AccessibilityShowsEveryToggle)
{
    MenuRig rig;
    client::Settings settings;

    const client::SettingsResult result =
        client::accessibilityPage(rig.context(), rig.cursor, settings);

    EXPECT_FALSE(result.changed);
    EXPECT_FALSE(result.back);
    EXPECT_EQ(rig.cursor.count, client::Toggles.size() + 1);
    for (const client::ToggleDef &toggle : client::Toggles) {
        const bool checked = settings.*toggle.field;
        EXPECT_TRUE(
            rig.drew((checked ? "[x] " : "[ ] ") + std::string{toggle.label}))
            << toggle.label;
    }
}

TEST(MenuPages, AccessibilityChangesTheSelectedToggleOnly)
{
    MenuRig rig;
    client::Settings settings;
    const client::Settings before;
    rig.cursor.selected = 1;

    const client::SettingsResult result = client::accessibilityPage(
        rig.context({.confirm = true}), rig.cursor, settings);

    EXPECT_TRUE(result.changed);
    EXPECT_NE(settings.*client::Toggles.at(1).field,
              before.*client::Toggles.at(1).field);
    EXPECT_EQ(settings.*client::Toggles.at(0).field,
              before.*client::Toggles.at(0).field);
    EXPECT_EQ(rig.played(), std::vector<std::string>{ConfirmSound});
}

TEST(MenuPages, AccessibilityGoesBack)
{
    MenuRig rig;
    client::Settings settings;
    rig.cursor.selected = client::Toggles.size();

    const client::SettingsResult result = client::accessibilityPage(
        rig.context({.confirm = true}), rig.cursor, settings);

    EXPECT_TRUE(result.back);
    EXPECT_FALSE(result.changed);
    EXPECT_EQ(rig.played(), std::vector<std::string>{BackSound});
}
