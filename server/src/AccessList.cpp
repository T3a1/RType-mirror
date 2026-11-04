/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** AccessList
*/

#include "AccessList.hpp"
#include "engine/Logger.hpp"
#include "engine/network/Endpoint.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace server {

namespace {

/**
 * @brief Reads a list of addresses, skipping blank lines and `#` comments.
 * @param path The file; a missing file gives an empty list.
 * @return The addresses.
 * @throws std::runtime_error If a line is not an IPv4 address.
 **/
std::set<std::uint32_t> readAddresses(const std::string &path)
{
    std::set<std::uint32_t> addresses;
    std::ifstream file(path);
    for (std::string line; std::getline(file, line);) {
        std::istringstream words(line.substr(0, line.find('#')));
        std::string word;
        if (!(words >> word)) {
            continue;
        }
        const auto address = engine::net::parseAddress(word);
        if (!address) {
            std::string message = path;
            message += ": not an IPv4 address: ";
            message += word;
            throw std::runtime_error(message);
        }
        addresses.insert(*address);
    }
    return addresses;
}

/**
 * @brief Rewrites a list of addresses; a failure is only logged, as the list
 * still applies in memory.
 * @param path The file; empty for none.
 * @param addresses The addresses.
 **/
void writeAddresses(const std::string &path,
                    const std::set<std::uint32_t> &addresses)
{
    if (path.empty()) {
        return;
    }
    std::ofstream file(path);
    for (const std::uint32_t address : addresses) {
        file << engine::net::addressToString(address) << '\n';
    }
    if (!file) {
        BB_ERROR("Could not write " << path);
    }
}

} // namespace

AccessList::AccessList(std::string banFile, std::string whitelistFile)
    : m_banFile(std::move(banFile)), m_whitelistFile(std::move(whitelistFile)),
      m_whitelistEnabled(!m_whitelistFile.empty())
{
    if (!m_banFile.empty()) {
        m_bans = readAddresses(m_banFile);
    }
    if (!m_whitelistFile.empty()) {
        m_allowed = readAddresses(m_whitelistFile);
    }
}

std::optional<rtype::protocol::RejectReason>
AccessList::check(std::uint32_t address) const
{
    if (m_bans.contains(address)) {
        return rtype::protocol::RejectReason::Banned;
    }
    if (m_whitelistEnabled && !m_allowed.contains(address)) {
        return rtype::protocol::RejectReason::NotWhitelisted;
    }
    return std::nullopt;
}

void AccessList::ban(std::uint32_t address)
{
    m_bans.insert(address);
    writeAddresses(m_banFile, m_bans);
}

bool AccessList::unban(std::uint32_t address)
{
    const bool removed = m_bans.erase(address) > 0;
    writeAddresses(m_banFile, m_bans);
    return removed;
}

void AccessList::allow(std::uint32_t address)
{
    m_allowed.insert(address);
    writeAddresses(m_whitelistFile, m_allowed);
}

bool AccessList::disallow(std::uint32_t address)
{
    const bool removed = m_allowed.erase(address) > 0;
    writeAddresses(m_whitelistFile, m_allowed);
    return removed;
}

} // namespace server
