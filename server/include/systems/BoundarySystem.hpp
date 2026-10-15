/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** BoundarySystem
*/

#ifndef SERVER_SYSTEMS_BOUNDARYSYSTEM_HPP
#define SERVER_SYSTEMS_BOUNDARYSYSTEM_HPP

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

#endif // SERVER_SYSTEMS_BOUNDARYSYSTEM_HPP
