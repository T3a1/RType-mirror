/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Piercing
*/

#ifndef SERVER_COMPONENTS_PIERCING_HPP
#define SERVER_COMPONENTS_PIERCING_HPP

#include "engine/Entity.hpp"

#include <vector>

namespace server {

/**
 * @struct Piercing
 * @brief A projectile that goes through what it hits instead of being
 * destroyed (a charged shot).
 * @param hit The entities it already damaged, each only once.
 **/
struct Piercing {
    std::vector<engine::Entity> hit;
};

} // namespace server

#endif // SERVER_COMPONENTS_PIERCING_HPP
