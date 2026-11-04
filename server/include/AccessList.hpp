/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** AccessList
*/

#ifndef SERVER_ACCESSLIST_HPP
#define SERVER_ACCESSLIST_HPP

#include "common/protocol/Messages.hpp"

#include <cstdint>
#include <optional>
#include <set>
#include <string>

namespace server {

/**
 * @class AccessList
 * @brief The IPv4 addresses banned from the server, and those allowed when it
 * only accepts listed addresses. A ban wins over the white list. Each list can
 * live in a text file, one address per line with `#` comments, read at startup
 * and rewritten at each change.
 *
 * Only one thread may use it: the network thread once the server runs.
 **/
class AccessList {
  public:
    /**
     * @brief Creates empty lists and an open server, not tied to files.
     **/
    AccessList() = default;

    /**
     * @brief Reads the lists from their files; a missing file is an empty
     * list.
     * @param banFile The file of the bans; empty for none.
     * @param whitelistFile The file of the allowed addresses; empty for none.
     * If given, the server starts with the white list on.
     * @throws std::runtime_error If a line is not an IPv4 address.
     **/
    AccessList(std::string banFile, std::string whitelistFile);

    /**
     * @brief Tells why an address may not connect.
     * @param address The address.
     * @return The reason, or nothing if it may connect.
     **/
    [[nodiscard]] std::optional<rtype::protocol::RejectReason>
    check(std::uint32_t address) const;

    /**
     * @brief Bans an address and saves the list.
     * @param address The address.
     **/
    void ban(std::uint32_t address);

    /**
     * @brief Removes a ban and saves the list.
     * @param address The address.
     * @return Whether it was banned.
     **/
    bool unban(std::uint32_t address);

    /**
     * @brief Allows an address and saves the list.
     * @param address The address.
     **/
    void allow(std::uint32_t address);

    /**
     * @brief Removes an allowed address and saves the list.
     * @param address The address.
     * @return Whether it was allowed.
     **/
    bool disallow(std::uint32_t address);

    /**
     * @brief Turns the white list on or off.
     * @param enabled Whether only listed addresses may connect.
     **/
    void setWhitelistEnabled(bool enabled) { m_whitelistEnabled = enabled; }

    /**
     * @brief Returns whether only listed addresses may connect.
     * @return True if the white list is on.
     **/
    [[nodiscard]] bool whitelistEnabled() const { return m_whitelistEnabled; }

    /**
     * @brief Returns the banned addresses.
     * @return The addresses.
     **/
    [[nodiscard]] const std::set<std::uint32_t> &bans() const { return m_bans; }

    /**
     * @brief Returns the allowed addresses.
     * @return The addresses.
     **/
    [[nodiscard]] const std::set<std::uint32_t> &allowed() const
    {
        return m_allowed;
    }

  private:
    std::string m_banFile;
    std::string m_whitelistFile;
    std::set<std::uint32_t> m_bans;
    std::set<std::uint32_t> m_allowed;
    bool m_whitelistEnabled = false;
};

} // namespace server

#endif // SERVER_ACCESSLIST_HPP
