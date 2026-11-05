/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** LobbySystem
*/

#include "systems/LobbySystem.hpp"
#include "PlayerColors.hpp"

#include <algorithm>
#include <array>
#include <string>

namespace client {

namespace {

using rtype::protocol::RoomInfo;
using rtype::protocol::RoomStatus;

/** @brief The width of a row of the screens, in pixels. */
constexpr float RowWidth = 520.0F;
/** @brief The space between two columns, in pixels. */
constexpr float ColumnGap = 12.0F;
/** @brief The height of a row, in pixels. */
constexpr float RowHeight = 40.0F;
/** @brief The space between two rows, in pixels. */
constexpr float RowGap = 6.0F;
/** @brief The space above the title, in pixels. */
constexpr float TopMargin = 24.0F;
/** @brief The height of the title, in pixels. */
constexpr float TitleHeight = 64.0F;
/** @brief The height of a line of text, in pixels. */
constexpr float LineHeight = 34.0F;
/** @brief The font size of the title, in pixels. */
constexpr int TitleFontSize = 48;
/** @brief The font size of the text, in pixels. */
constexpr int TextFontSize = 24;
/** @brief The items before the rooms of the list: Quick match and New room. */
constexpr std::size_t FixedItems = 2;

/**
 * @brief Moves the keyboard selection, wrapping around.
 * @param selected The selected item, updated.
 * @param count The number of items.
 * @param keys The menu keys of this frame.
 **/
void moveSelection(std::size_t &selected, std::size_t count,
                   const MenuKeys &keys)
{
    selected = std::min(selected, count - 1);
    if (keys.up) {
        selected = (selected + count - 1) % count;
    }
    if (keys.down) {
        selected = (selected + 1) % count;
    }
}

/**
 * @brief Returns the label of a room in the menu.
 * @param room The room.
 * @return Its number, its players and where its game stands.
 **/
std::string roomLabel(const RoomInfo &room)
{
    std::string status = "waiting";
    if (room.status == RoomStatus::Playing) {
        status = "playing";
    } else if (room.status == RoomStatus::Over) {
        status = "game over";
    }
    return "Room " + std::to_string(room.room) + "  " +
           std::to_string(room.players) + "/" +
           std::to_string(rtype::protocol::MaxPlayers) + "  " + status;
}

/**
 * @brief Returns the rectangle of a row of a centered column.
 * @param screen The size of the window.
 * @param y The top of the row.
 * @return The rectangle.
 **/
gfx::Rect rowAt(engine::Vec2f screen, float y)
{
    return {
        .x = (screen.x - RowWidth) / 2.0F,
        .y = y,
        .width = RowWidth,
        .height = RowHeight,
    };
}

/**
 * @brief Returns the rectangle of an item of the lobby: two columns, so that
 * the 16 rooms fit in a 720-pixel window.
 * @param screen The size of the window.
 * @param top The top of the first row.
 * @param index The index of the item.
 * @return The rectangle.
 **/
gfx::Rect itemAt(engine::Vec2f screen, float top, std::size_t index)
{
    const float left = (screen.x - (2.0F * RowWidth) - ColumnGap) / 2.0F;
    const std::size_t column = index % 2;
    const std::size_t row = index / 2;
    return {
        .x = left + (static_cast<float>(column) * (RowWidth + ColumnGap)),
        .y = top + (static_cast<float>(row) * (RowHeight + RowGap)),
        .width = RowWidth,
        .height = RowHeight,
    };
}

} // namespace

bool canJoin(const RoomInfo &room)
{
    return room.status != RoomStatus::Over &&
           room.players < rtype::protocol::MaxPlayers;
}

std::optional<std::uint32_t>
lobbyScreen(LobbyState &state, gfx::IGraphics &graphics, const MenuKeys &keys)
{
    const engine::Vec2f screen = graphics.screenSize();
    const std::size_t count = FixedItems + state.rooms.size();
    moveSelection(state.selected, count, keys);
    const bool joining = state.screen == LobbyState::Screen::Joining;

    float y = TopMargin;
    drawCentered(graphics, "R-TYPE", screen.x / 2.0F, y, TitleFontSize,
                 gfx::colors::OffWhite);
    y += TitleHeight;
    std::string hint = "Up, Down and Space, or the mouse. Esc quits.";
    if (joining) {
        hint = "Joining...";
    } else if (state.refused) {
        hint = "This room is not available anymore.";
    } else if (state.closed) {
        hint = "An administrator closed your room.";
    }
    drawCentered(graphics, hint, screen.x / 2.0F, y, TextFontSize,
                 gfx::colors::OffWhite);
    y += LineHeight;

    std::optional<std::uint32_t> choice;
    for (std::size_t i = 0; i < count; ++i) {
        std::string label = "Quick match";
        std::uint32_t room = rtype::protocol::AnyRoom;
        bool enabled = !joining;
        if (i == 1) {
            label = "New room";
            room = rtype::protocol::NewRoom;
        } else if (i >= FixedItems) {
            const RoomInfo &info = state.rooms[i - FixedItems];
            label = roomLabel(info);
            room = info.room;
            enabled = enabled && canJoin(info);
        }
        const bool selected = i == state.selected;
        const bool clicked =
            menuButton(graphics, itemAt(screen, y, i), label,
                       {.selected = selected, .enabled = enabled});
        if (clicked || (selected && enabled && keys.confirm)) {
            choice = room;
        }
    }
    return choice;
}

namespace {

// Items of the waiting room: the settings, then Start and Leave.
constexpr std::size_t SettingCount = 3;
constexpr std::size_t StartItem = SettingCount;
constexpr std::size_t LeaveItem = SettingCount + 1;
constexpr std::size_t WaitingItems = SettingCount + 2;
constexpr gfx::Color HintColor = gfx::rgb(170, 170, 185);

std::string difficultyName(rtype::protocol::Difficulty difficulty)
{
    switch (difficulty) {
    case rtype::protocol::Difficulty::Easy:
        return "Easy";
    case rtype::protocol::Difficulty::Normal:
        return "Normal";
    case rtype::protocol::Difficulty::Hard:
        return "Hard";
    }
    return "?";
}

std::string levelName(std::uint8_t level)
{
    return level == rtype::protocol::EndlessLevel
               ? std::string{"Endless mode"}
               : "Level " + std::to_string(level);
}

std::string bossName(rtype::protocol::BossChoice boss)
{
    switch (boss) {
    case rtype::protocol::BossChoice::LevelBoss:
        return "Each level's own";
    case rtype::protocol::BossChoice::Warship:
        return "Warship";
    case rtype::protocol::BossChoice::Hive:
        return "Hive";
    }
    return "?";
}

/**
 * @brief Moves one setting of the waiting room to its next value, back to
 * the first after the last.
 * @param state The lobby state holding the settings.
 * @param setting The setting: 0 difficulty, 1 starting level, 2 boss.
 **/
void nextSetting(LobbyState &state, std::size_t setting)
{
    rtype::protocol::StartGame &settings = state.settings;
    switch (setting) {
    case 0:
        settings.difficulty = static_cast<rtype::protocol::Difficulty>(
            (static_cast<int>(settings.difficulty) + 1) % 3);
        break;
    case 1:
        // Level 1, 2... up to the last one, then the endless mode.
        if (settings.level == rtype::protocol::EndlessLevel) {
            settings.level =
                state.levels > 0 ? 1 : rtype::protocol::EndlessLevel;
        } else if (settings.level >= state.levels) {
            settings.level = rtype::protocol::EndlessLevel;
        } else {
            ++settings.level;
        }
        break;
    default:
        settings.boss = static_cast<rtype::protocol::BossChoice>(
            (static_cast<int>(settings.boss) + 1) % 3);
        break;
    }
}

} // namespace

WaitingChoice waitingRoomScreen(LobbyState &state, const SyncState &sync,
                                gfx::IGraphics &graphics, const MenuKeys &keys)
{
    const engine::Vec2f screen = graphics.screenSize();
    moveSelection(state.selected, WaitingItems, keys);

    float y = TopMargin;
    drawCentered(graphics, "Room " + std::to_string(state.room),
                 screen.x / 2.0F, y, TitleFontSize, gfx::colors::OffWhite);
    y += TitleHeight;
    for (const auto &player : sync.players) {
        const bool local = player.slot == sync.localSlot;
        drawCentered(
            graphics,
            "P" + std::to_string(player.slot) + (local ? "  (you)" : ""),
            screen.x / 2.0F, y, TextFontSize, playerColor(player.slot));
        y += LineHeight;
    }

    // Each setting is a button that goes to its next value.
    const std::array<std::string, SettingCount> labels{
        "Difficulty: " + difficultyName(state.settings.difficulty),
        "Start at: " + levelName(state.settings.level),
        "Boss: " + bossName(state.settings.boss),
    };
    for (std::size_t i = 0; i < SettingCount; ++i) {
        const bool clicked =
            menuButton(graphics, rowAt(screen, y), labels.at(i),
                       {.selected = state.selected == i});
        if (clicked || (keys.confirm && state.selected == i)) {
            nextSetting(state, i);
        }
        y += RowHeight + RowGap;
    }
    drawCentered(graphics, "The settings of the player who starts are used",
                 screen.x / 2.0F, y, TextFontSize, HintColor);
    y += LineHeight;

    const bool start = menuButton(graphics, rowAt(screen, y), "Start the game",
                                  {.selected = state.selected == StartItem});
    y += RowHeight + RowGap;
    const bool leave = menuButton(graphics, rowAt(screen, y), "Back to lobby",
                                  {.selected = state.selected == LeaveItem});
    if (start || (keys.confirm && state.selected == StartItem)) {
        return WaitingChoice::Start;
    }
    if (leave || (keys.confirm && state.selected == LeaveItem)) {
        return WaitingChoice::Leave;
    }
    return WaitingChoice::None;
}

} // namespace client
