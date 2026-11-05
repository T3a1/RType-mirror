/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** ExecutableDirectory
*/

#include "ExecutableDirectory.hpp"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <array>
#include <system_error>

namespace client::gfx {

std::filesystem::path executableDirectory()
{
#ifdef _WIN32
    std::array<wchar_t, MAX_PATH> buffer{};
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(),
                                            static_cast<DWORD>(buffer.size()));
    if (length == 0 || length == buffer.size()) {
        return {};
    }
    return std::filesystem::path{buffer.data()}.parent_path();
#else
    std::error_code error;
    const std::filesystem::path executable =
        std::filesystem::read_symlink("/proc/self/exe", error);
    return error ? std::filesystem::path{} : executable.parent_path();
#endif
}

} // namespace client::gfx
