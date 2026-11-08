/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** ExecutableDirectory
*/

#ifndef CLIENT_GRAPHICS_EXECUTABLEDIRECTORY_HPP
#define CLIENT_GRAPHICS_EXECUTABLEDIRECTORY_HPP

#include <filesystem>

namespace client::gfx {

/**
 * @brief The directory holding the running executable.
 * @return The directory, or an empty path (the current directory) if it
 * cannot be found.
 */
[[nodiscard]] std::filesystem::path executableDirectory();

} // namespace client::gfx

#endif // CLIENT_GRAPHICS_EXECUTABLEDIRECTORY_HPP
