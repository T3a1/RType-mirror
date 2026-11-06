/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** menu_test
*/

#include "FakeAudio.hpp"
#include "Menu.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

namespace {

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

    // One frame of a menu of a few buttons; returns which were chosen.
    std::vector<bool> buttons(std::size_t count, const MenuKeys &keys = {})
    {
        MenuPanel panel(graphics, keys, board, cursor);
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

    MenuPanel panel(rig.graphics, {}, rig.board, rig.cursor);

    EXPECT_TRUE(panel.button(Row, "Item"));
    EXPECT_EQ(rig.played(), std::vector<std::string>{ConfirmSound});
}

TEST(MenuPanel, ADisabledButtonCannotBeChosen)
{
    MenuRig rig;
    rig.click(Row.x + 1.0F);

    MenuPanel panel(rig.graphics, {.confirm = true}, rig.board, rig.cursor);

    EXPECT_FALSE(panel.button(Row, "Item", false));
    EXPECT_TRUE(rig.played().empty());
}

TEST(MenuPanel, ABackButtonPlaysTheBackSound)
{
    MenuRig rig;

    MenuPanel panel(rig.graphics, {.confirm = true}, rig.board, rig.cursor);

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
    MenuPanel panel(rig.graphics, {}, rig.board, rig.cursor);
    static_cast<void>(panel.button(Row, "First"));
    EXPECT_FALSE(rig.drew(">"));
}

TEST(MenuPanel, ConfirmTogglesACheckbox)
{
    MenuRig rig;
    bool value = false;

    MenuPanel panel(rig.graphics, {.confirm = true}, rig.board, rig.cursor);

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

    MenuPanel panel(rig.graphics, {}, rig.board, rig.cursor);

    EXPECT_TRUE(panel.checkbox(Row, "Labels", value));
    EXPECT_FALSE(value);
    EXPECT_TRUE(rig.drew("[ ] Labels"));
}

TEST(MenuPanel, ACheckboxStaysAloneWithoutInput)
{
    MenuRig rig;
    bool value = true;

    MenuPanel panel(rig.graphics, {}, rig.board, rig.cursor);

    EXPECT_FALSE(panel.checkbox(Row, "Labels", value));
    EXPECT_TRUE(value);
    EXPECT_TRUE(rig.played().empty());
}

TEST(MenuPanel, LeftAndRightMoveASliderBySteps)
{
    MenuRig rig;
    float value = 0.4F;
    {
        MenuPanel panel(rig.graphics, {.right = true}, rig.board, rig.cursor);
        EXPECT_TRUE(panel.slider(Row, "Music", value));
    }
    EXPECT_FLOAT_EQ(value, 0.45F);
    {
        MenuPanel panel(rig.graphics, {.left = true}, rig.board, rig.cursor);
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
        MenuPanel panel(rig.graphics, {.left = true}, rig.board, rig.cursor);
        EXPECT_FALSE(panel.slider(Row, "Low", low));
    }
    {
        MenuPanel panel(rig.graphics, {.right = true}, rig.board, rig.cursor);
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

    MenuPanel panel(rig.graphics, {.right = true}, rig.board, rig.cursor);
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

    MenuPanel panel(rig.graphics, {}, rig.board, rig.cursor);

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

    MenuPanel panel(rig.graphics, {}, rig.board, rig.cursor);

    EXPECT_FALSE(panel.slider(Row, "Music", value));
    EXPECT_FLOAT_EQ(value, 0.5F);
}

TEST(MenuPanel, LeftAndRightChangeTheTabAndWrap)
{
    MenuRig rig;
    const std::array<std::string, 2> labels{"Audio", "Controls"};
    std::size_t current = 0;
    {
        MenuPanel panel(rig.graphics, {.left = true}, rig.board, rig.cursor);
        panel.tabs(Row, labels, current);
    }
    EXPECT_EQ(current, 1U);
    EXPECT_TRUE(rig.drew("[Controls]"));
    {
        MenuPanel panel(rig.graphics, {.right = true}, rig.board, rig.cursor);
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

    MenuPanel panel(rig.graphics, {.right = true}, rig.board, rig.cursor);
    panel.tabs(Row, labels, current);

    EXPECT_EQ(current, 0U);
}

TEST(MenuPanel, ClickChoosesATab)
{
    MenuRig rig;
    const std::array<std::string, 2> labels{"Audio", "Controls"};
    std::size_t current = 0;
    rig.click(Row.x + Row.width - 1.0F);

    MenuPanel panel(rig.graphics, {}, rig.board, rig.cursor);
    panel.tabs(Row, labels, current);

    EXPECT_EQ(current, 1U);
    EXPECT_EQ(rig.played(), std::vector<std::string>{MoveSound});
}

TEST(MenuPanel, EmptyTabsStillCountAsAWidget)
{
    MenuRig rig;
    std::size_t current = 3;
    {
        MenuPanel panel(rig.graphics, {}, rig.board, rig.cursor);
        panel.tabs(Row, {}, current);
    }
    EXPECT_EQ(rig.cursor.count, 1U);
    EXPECT_EQ(current, 3U);
}
