/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** RoomPages
*/

#include "RoomPages.hpp"
#include "PlayerColors.hpp"

#include <array>
#include <string>

namespace client {

namespace {

using rtype::protocol::RoomInfo;
using rtype::protocol::RoomStatus;

/** @brief The items before the rooms of the list: Quick match and New room. */
constexpr std::size_t FixedItems = 2;

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

} // namespace

bool canJoin(const RoomInfo &room)
{
    return room.status != RoomStatus::Over &&
           room.players < rtype::protocol::MaxPlayers;
}

std::optional<std::uint32_t> lobbyScreen(LobbyState &state,
                                         const MenuContext &context)
{
    MenuPanel panel(context, state.lobbyCursor);
    gfx::IGraphics &graphics = *context.graphics;
    const engine::Vec2f screen = graphics.screenSize();
    const bool joining = state.screen == LobbyState::Screen::Joining;

    float y = drawMenuTitle(graphics, "R-TYPE");
    std::string hint = "Up, Down and Enter, or the mouse. Esc goes back.";
    if (joining) {
        hint = "Joining...";
    } else if (state.refused) {
        hint = "This room is not available anymore.";
    } else if (state.closed) {
        hint = "An administrator closed your room.";
    }
    drawCentered(graphics, hint, screen.x / 2.0F, y, MenuTextFontSize,
                 gfx::colors::OffWhite);
    y += MenuLineHeight;

    std::optional<std::uint32_t> choice;
    if (panel.button(menuGridCell(screen, y, 0), "Quick match", !joining)) {
        choice = rtype::protocol::AnyRoom;
    }
    if (panel.button(menuGridCell(screen, y, 1), "New room", !joining)) {
        choice = rtype::protocol::NewRoom;
    }
    std::size_t index = FixedItems;
    for (const RoomInfo &room : state.rooms) {
        if (panel.button(menuGridCell(screen, y, index), roomLabel(room),
                         !joining && canJoin(room))) {
            choice = room.room;
        }
        ++index;
    }
    return choice;
}

namespace {

/** @brief The settings of the waiting room: difficulty, level and boss. */
constexpr std::size_t SettingCount = 3;
/** @brief The color of the hints of the waiting room. */
constexpr gfx::Color HintColor = gfx::rgb(170, 170, 185);

/**
 * @brief Returns the text of a difficulty.
 * @param difficulty The difficulty.
 * @return Its name, as shown in the waiting room.
 **/
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

/**
 * @brief Returns the text of a starting level.
 * @param level The level, from 1, or the endless mode.
 * @return Its name, as shown in the waiting room.
 **/
std::string levelName(std::uint8_t level)
{
    return level == rtype::protocol::EndlessLevel
               ? std::string{"Endless mode"}
               : "Level " + std::to_string(level);
}

/**
 * @brief Returns the text of a boss choice.
 * @param boss The choice.
 * @return Its name, as shown in the waiting room.
 **/
std::string bossName(rtype::protocol::BossChoice boss)
{
    switch (boss) {
    case rtype::protocol::BossChoice::LevelBoss:
        return "Each level's own";
    case rtype::protocol::BossChoice::Warship:
        return "Warship";
    case rtype::protocol::BossChoice::Hive:
        return "Hive";
    case rtype::protocol::BossChoice::Walker:
        return "Walker";
    case rtype::protocol::BossChoice::Twin:
        return "Twin";
    case rtype::protocol::BossChoice::Behemoth:
        return "Behemoth";
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
            (static_cast<int>(settings.boss) + 1) %
            rtype::protocol::BossChoiceCount);
        break;
    }
}

} // namespace

WaitingChoice waitingRoomScreen(LobbyState &state, const SyncState &sync,
                                const MenuContext &context)
{
    MenuPanel panel(context, state.waitingCursor);
    gfx::IGraphics &graphics = *context.graphics;
    const engine::Vec2f screen = graphics.screenSize();

    float y = drawMenuTitle(graphics, "Room " + std::to_string(state.room));
    for (const auto &player : sync.players) {
        const bool local = player.slot == sync.localSlot;
        drawCentered(
            graphics,
            "P" + std::to_string(player.slot) + (local ? "  (you)" : ""),
            screen.x / 2.0F, y, MenuTextFontSize, playerColor(player.slot));
        y += MenuLineHeight;
    }

    // Each setting is a button that goes to its next value.
    const std::array<std::string, SettingCount> labels{
        "Difficulty: " + difficultyName(state.settings.difficulty),
        "Start at: " + levelName(state.settings.level),
        "Boss: " + bossName(state.settings.boss),
    };
    for (std::size_t i = 0; i < SettingCount; ++i) {
        if (panel.button(menuRow(screen, y), labels.at(i))) {
            nextSetting(state, i);
        }
        y += MenuRowHeight + MenuRowGap;
    }
    drawCentered(graphics, "The settings of the player who starts are used",
                 screen.x / 2.0F, y, MenuTextFontSize, HintColor);
    y += MenuLineHeight;

    const bool start = panel.button(menuRow(screen, y), "Start the game");
    y += MenuRowHeight + MenuRowGap;
    const bool leave = panel.backButton(menuRow(screen, y), "Back to lobby");
    if (start) {
        return WaitingChoice::Start;
    }
    return leave ? WaitingChoice::Leave : WaitingChoice::None;
}

} // namespace client
