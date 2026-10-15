/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BoundarySystem
*/

#ifndef SERVER_BOUNDARY_SYSTEM_HPP
#define SERVER_BOUNDARY_SYSTEM_HPP

#include "common/Playfield.hpp"
#include "common/components/Collider.hpp"
#include "common/components/Transform.hpp"
#include "components/DestroyOffScreen.hpp"
#include "components/KeepInPlayfield.hpp"
#include "engine/Entity.hpp"
#include "engine/Logger.hpp"
#include "engine/Registry.hpp"

#include <algorithm>

namespace server {

void boundarySystem(engine::Registry &registry);

} // namespace server

#endif // SERVER_BOUNDARY_SYSTEM_HPP
