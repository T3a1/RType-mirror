/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** LobbySystem
*/

#ifndef CLIENT_SYSTEMS_LOBBYSYSTEM_HPP
#define CLIENT_SYSTEMS_LOBBYSYSTEM_HPP

#include "Menu.hpp"
#include "common/protocol/Messages.hpp"
#include "graphics/IGraphics.hpp"
#include "systems/NetworkSyncSystem.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace client {

/**
 * @struct LobbyState
 * @brief What the client knows about the lobby and its current room.
 * @param screen Where the player is.
 * @param rooms The last list of the rooms from the server.
 * @param room The current room, for the InRoom screen.
 * @param refused Whether the server refused the last room asked for.
 * @param selected The item of the menu selected with the keyboard.
 **/
struct LobbyState {
    /**
     * @enum Screen
     * @brief Where the player is.
     * @param Lobby In the menu of the rooms.
     * @param Joining A room was asked for, waiting for the answer.
     * @param InRoom In a room, waiting for its game or playing it.
     **/
    enum class Screen : std::uint8_t { Lobby, Joining, InRoom };

    Screen screen = Screen::Lobby;
    std::vector<rtype::protocol::RoomInfo> rooms;
    std::uint32_t room = rtype::protocol::AnyRoom;
    bool refused = false;
    std::size_t selected = 0;
};

/**
 * @enum WaitingChoice
 * @brief What the player chose in a waiting room.
 * @param None Nothing this frame.
 * @param Start Start the game.
 * @param Leave Go back to the lobby.
 **/
enum class WaitingChoice : std::uint8_t { None, Start, Leave };

/**
 * @brief Returns whether a room of the list can be entered.
 * @param room The room.
 * @return False if it is full or its game is over.
 **/
[[nodiscard]] bool canJoin(const rtype::protocol::RoomInfo &room);

/**
 * @brief Draws the menu of the rooms, in screen pixels, and handles its mouse
 * and keyboard choices: quick match, new room, or a room of the list.
 * @param state The lobby state; the keyboard selection is updated.
 * @param graphics The graphics backend.
 * @param keys The menu keys of this frame.
 * @return The room to ask for (or AnyRoom or NewRoom), if one was chosen.
 **/
[[nodiscard]] std::optional<std::uint32_t>
lobbyScreen(LobbyState &state, gfx::IGraphics &graphics, const MenuKeys &keys);

/**
 * @brief Draws the panel of a waiting room, in screen pixels: its players and
 * the Start and Leave buttons.
 * @param state The lobby state; the keyboard selection is updated.
 * @param sync The sync state, for the players of the room.
 * @param graphics The graphics backend.
 * @param keys The menu keys of this frame.
 * @return What the player chose.
 **/
[[nodiscard]] WaitingChoice waitingRoomScreen(LobbyState &state,
                                              const SyncState &sync,
                                              gfx::IGraphics &graphics,
                                              const MenuKeys &keys);

} // namespace client

#endif // CLIENT_SYSTEMS_LOBBYSYSTEM_HPP
