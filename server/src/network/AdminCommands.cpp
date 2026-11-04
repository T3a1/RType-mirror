/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** AdminCommands
*/

#include "network/NetworkServer.hpp"

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace server {

namespace {

using Args = engine::CommandConsole::Args;

constexpr int IdWidth = 6;
constexpr int AddressWidth = 24;
constexpr int RoomWidth = 8;
constexpr int StatusWidth = 10;

/**
 * @brief Reads a number argument.
 * @param text The argument.
 * @return The number.
 * @throws engine::CommandConsole::UsageError If it is not a positive number.
 **/
std::uint32_t numberArgument(const std::string &text)
{
    const std::size_t digits = text.find_first_not_of("0123456789");
    if (text.empty() || digits != std::string::npos || text.size() > 9) {
        throw engine::CommandConsole::UsageError();
    }
    return static_cast<std::uint32_t>(std::stoul(text));
}

/**
 * @brief Reads an IPv4 address argument.
 * @param text The argument.
 * @return The address.
 * @throws engine::CommandConsole::UsageError If it is not an address.
 **/
std::uint32_t addressArgument(const std::string &text)
{
    const auto address = engine::net::parseAddress(text);
    if (!address) {
        throw engine::CommandConsole::UsageError();
    }
    return *address;
}

/**
 * @brief Returns the name of a room status.
 * @param status The status, as given by the game.
 * @return Its name.
 **/
std::string_view statusName(std::uint8_t status)
{
    switch (static_cast<rtype::protocol::RoomStatus>(status)) {
    case rtype::protocol::RoomStatus::Waiting:
        return "waiting";
    case rtype::protocol::RoomStatus::Playing:
        return "playing";
    case rtype::protocol::RoomStatus::Over:
        return "over";
    }
    return "unknown";
}

/**
 * @brief Writes a count and its noun, in the singular or the plural.
 * @param count The count.
 * @param noun The noun in the singular.
 * @return For example "1 client" or "2 clients".
 **/
std::string countOf(int count, const std::string &noun)
{
    return std::to_string(count) + " " + noun + (count == 1 ? "" : "s");
}

/**
 * @brief Lists addresses, one per line.
 * @param addresses The addresses.
 * @param empty The text if there are none.
 * @return The text.
 **/
std::string addressList(const std::set<std::uint32_t> &addresses,
                        const std::string &empty)
{
    if (addresses.empty()) {
        return empty;
    }
    std::string text;
    for (const std::uint32_t address : addresses) {
        if (!text.empty()) {
            text += '\n';
        }
        text += engine::net::addressToString(address);
    }
    return text;
}

} // namespace

std::future<std::string> submitCommand(AdminQueue &queue, std::string line)
{
    AdminRequest request{.line = std::move(line), .answer = {}};
    auto answer = request.answer.get_future();
    queue.push(std::move(request));
    return answer;
}

void NetworkServer::runAdminRequests()
{
    for (AdminRequest &request : m_adminQueue->drain()) {
        request.answer.set_value(m_commands.execute(request.line));
    }
}

void NetworkServer::registerCommands()
{
    m_commands.add("status", "", "Rooms, clients and access rules.",
                   [this](const Args &) { return statusCommand(); });
    m_commands.add("rooms", "", "List the rooms.",
                   [this](const Args &) { return roomsCommand(); });
    m_commands.add("players", "[room]", "List the clients, of one room or all.",
                   [this](const Args &args) { return playersCommand(args); });
    m_commands.add("kick", "<id>", "Disconnect a client.",
                   [this](const Args &args) { return kickCommand(args); });
    m_commands.add("ban", "<id|ip>",
                   "Ban an address and disconnect its clients.",
                   [this](const Args &args) { return banCommand(args); });
    m_commands.add("unban", "<ip>", "Remove a ban.",
                   [this](const Args &args) { return unbanCommand(args); });
    m_commands.add("banlist", "", "List the banned addresses.",
                   [this](const Args &) { return banlistCommand(); });
    m_commands.add("stop-room", "<room>",
                   "Close a room; its players go back to the lobby.",
                   [this](const Args &args) { return stopRoomCommand(args); });
    m_commands.add("whitelist", "on|off|list|add <ip>|remove <ip>",
                   "Only accept listed addresses.",
                   [this](const Args &args) { return whitelistCommand(args); });
}

std::string NetworkServer::statusCommand() const
{
    const auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::steady_clock::now() - m_startTime)
                            .count();
    std::size_t inLobby = 0;
    for (const auto &[endpoint, session] : m_sessions) {
        inLobby += session.link == nullptr ? 1 : 0;
    }
    std::ostringstream out;
    out << "Uptime   " << uptime << " s\n"
        << "Rooms    " << m_rooms.size() << " / "
        << int{rtype::protocol::MaxRooms} << '\n'
        << "Clients  " << m_sessions.size() << " (" << inLobby
        << " in the lobby)\n"
        << "Access   "
        << (m_access.whitelistEnabled() ? "listed addresses only" : "open")
        << ", " << m_access.bans().size() << " banned";
    return out.str();
}

std::string NetworkServer::roomsCommand() const
{
    std::ostringstream out;
    out << std::left << std::setw(RoomWidth) << "ROOM" << std::setw(StatusWidth)
        << "STATUS" << "PLAYERS";
    for (const engine::InstanceInfo &info : m_rooms.list()) {
        out << '\n'
            << std::setw(RoomWidth) << info.id << std::setw(StatusWidth)
            << statusName(info.status) << info.members << '/'
            << int{rtype::protocol::MaxPlayers};
    }
    return out.str();
}

std::string NetworkServer::playersCommand(const Args &args) const
{
    if (args.size() > 1) {
        throw engine::CommandConsole::UsageError();
    }
    const std::optional<std::uint32_t> room =
        args.empty() ? std::nullopt
                     : std::optional{numberArgument(args.front())};
    const auto now = std::chrono::steady_clock::now();
    std::ostringstream out;
    out << std::left << std::setw(IdWidth) << "ID" << std::setw(AddressWidth)
        << "ADDRESS" << std::setw(RoomWidth) << "ROOM" << "LAST HEARD";
    for (const auto &[endpoint, session] : m_sessions) {
        if (room && session.room != *room) {
            continue;
        }
        const std::chrono::duration<double> silence = now - session.lastHeard;
        out << '\n'
            << std::setw(IdWidth) << session.id << std::setw(AddressWidth)
            << endpoint.toString() << std::setw(RoomWidth)
            << (session.link == nullptr ? std::string{"lobby"}
                                        : std::to_string(session.room))
            << std::fixed << std::setprecision(1) << silence.count() << " s";
    }
    return out.str();
}

void NetworkServer::kick(SessionIterator it)
{
    sendPacket(it->first,
               {
                   .payload =
                       rtype::protocol::Disconnect{
                           .reason = rtype::protocol::DisconnectReason::Kicked,
                       },
               });
    it->second.kicked = true;
}

std::string NetworkServer::kickCommand(const Args &args)
{
    if (args.size() != 1) {
        throw engine::CommandConsole::UsageError();
    }
    const std::uint32_t id = numberArgument(args.front());
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        if (it->second.id == id) {
            kick(it);
            return "Client " + args.front() + " kicked";
        }
    }
    return "No client " + args.front();
}

std::string NetworkServer::banCommand(const Args &args)
{
    if (args.size() != 1) {
        throw engine::CommandConsole::UsageError();
    }
    std::optional<std::uint32_t> address =
        engine::net::parseAddress(args.front());
    if (!address) {
        const std::uint32_t id = numberArgument(args.front());
        for (const auto &[endpoint, session] : m_sessions) {
            if (session.id == id) {
                address = endpoint.address;
            }
        }
    }
    if (!address) {
        return "No client " + args.front();
    }
    m_access.ban(*address);
    int kicked = 0;
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        if (it->first.address == *address) {
            kick(it);
            ++kicked;
        }
    }
    return engine::net::addressToString(*address) + " banned, " +
           countOf(kicked, "client") + " disconnected";
}

std::string NetworkServer::unbanCommand(const Args &args)
{
    if (args.size() != 1) {
        throw engine::CommandConsole::UsageError();
    }
    return args.front() + (m_access.unban(addressArgument(args.front()))
                               ? " unbanned"
                               : " was not banned");
}

std::string NetworkServer::banlistCommand() const
{
    return addressList(m_access.bans(), "No banned address");
}

std::string NetworkServer::stopRoomCommand(const Args &args)
{
    if (args.size() != 1) {
        throw engine::CommandConsole::UsageError();
    }
    const std::uint32_t room = numberArgument(args.front());
    const auto now = std::chrono::steady_clock::now();
    int moved = 0;
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        if (it->second.link != nullptr && it->second.room == room) {
            send(it, rtype::protocol::RoomClosed{}, now);
            leaveRoom(it, rtype::protocol::DisconnectReason::Kicked);
            sendRoomList(it, now);
            ++moved;
        }
    }
    if (moved == 0) {
        return "No room " + args.front();
    }
    return "Room " + args.front() + " closed, " + countOf(moved, "player") +
           " back in the lobby";
}

std::string NetworkServer::whitelistCommand(const Args &args)
{
    if (args.size() == 1 && args.front() == "on") {
        m_access.setWhitelistEnabled(true);
        return "Only listed addresses can connect now";
    }
    if (args.size() == 1 && args.front() == "off") {
        m_access.setWhitelistEnabled(false);
        return "Every address can connect now";
    }
    if (args.size() == 1 && args.front() == "list") {
        return addressList(m_access.allowed(), "No listed address");
    }
    if (args.size() == 2 && args.front() == "add") {
        m_access.allow(addressArgument(args.back()));
        return args.back() + " added";
    }
    if (args.size() == 2 && args.front() == "remove") {
        return args.back() + (m_access.disallow(addressArgument(args.back()))
                                  ? " removed"
                                  : " was not listed");
    }
    throw engine::CommandConsole::UsageError();
}

} // namespace server
