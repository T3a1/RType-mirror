/*
** EPITECH PROJECT, 2026
** r-type_common
** File description:
** NetworkId
*/

#ifndef COMMON_COMPONENTS_NETWORKID_HPP
#define COMMON_COMPONENTS_NETWORKID_HPP

#include <cstdint>

namespace rtype {

// Id of an entity shared by the server and the clients. Entity handles
// only mean something inside one registry; this one is the same
// everywhere. The server never reuses a NetworkId.
struct NetworkId {
    std::uint32_t value = 0;
};

} // namespace rtype

#endif // COMMON_COMPONENTS_NETWORKID_HPP
