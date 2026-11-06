/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Obstacle
*/

#ifndef SERVER_COMPONENTS_OBSTACLE_HPP
#define SERVER_COMPONENTS_OBSTACLE_HPP

namespace server {

/**
 * @struct Obstacle
 * @brief A part of the level scrolling to the left: it stops the shots and
 * destroys the ships that touch it.
 * @param destroyable Whether it is a tile that breaks once its Health is
 * gone, instead of a wall that nothing breaks.
 **/
struct Obstacle {
    bool destroyable = false;
};

} // namespace server

#endif // SERVER_COMPONENTS_OBSTACLE_HPP
