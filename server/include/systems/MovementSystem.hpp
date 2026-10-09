/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** MovementSystem
*/

#ifndef SERVER_SYSTEMS_MOVEMENTSYSTEM_HPP
#define SERVER_SYSTEMS_MOVEMENTSYSTEM_HPP

#include "engine/Clock.hpp"
#include "engine/Registry.hpp"

namespace server {

void movementSystem(engine::Registry &registry, engine::Duration dt);

} // namespace server

#endif // SERVER_SYSTEMS_MOVEMENTSYSTEM_HPP
