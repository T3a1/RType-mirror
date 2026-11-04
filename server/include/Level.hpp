/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Level
*/

#ifndef SERVER_LEVEL_HPP
#define SERVER_LEVEL_HPP

#include "Formations.hpp"
#include "engine/Clock.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace server {

/** @brief Level files larger than this are not read, in bytes. */
inline constexpr std::uintmax_t MaxLevelFileSize = 1U << 20U;
/** @brief The most waves one level can have. */
inline constexpr std::size_t MaxWaves = 1000;
/** @brief The most enemies one wave can have. */
inline constexpr std::size_t MaxWaveCount = 20;
/** @brief The latest time a wave can start, in seconds. */
inline constexpr float MaxWaveTime = 3600.0F;
/** @brief The longest level name, in characters. */
inline constexpr std::size_t MaxLevelNameLength = 64;

/**
 * @struct Wave
 * @brief A formation that arrives at a given time of a level.
 * @param time When it starts, counted from the start of the level.
 * @param formation The enemies and how they are laid out.
 * @param centerY The height of its center, in playfield units, or nothing
 * for a random height that keeps it inside the playfield.
 **/
struct Wave {
    engine::Duration time{0.0F};
    Formation formation;
    std::optional<float> centerY;
};

/**
 * @struct Level
 * @brief A level, as read from its JSON file.
 * @param name The name shown to the players.
 * @param waves Its waves, sorted by time.
 **/
struct Level {
    std::string name;
    std::vector<Wave> waves;
};

/**
 * @brief Reads a level from the text of a JSON file. See server/levels/ and
 * docs/LEVELS.md for the format. Unknown keys and out-of-range values are
 * errors, so that typos are reported instead of ignored.
 * @param text The content of the file.
 * @param error Set to what is wrong when the level cannot be read.
 * @return The level, or nothing if the text is not a valid level.
 **/
[[nodiscard]] std::optional<Level> parseLevel(std::string_view text,
                                              std::string &error);

/**
 * @struct LevelSet
 * @brief The levels of a directory and the files that could not be read.
 * @param levels The valid levels, in the order of their file names.
 * @param errors One message per file that was skipped, starting with its
 * name.
 **/
struct LevelSet {
    std::vector<Level> levels;
    std::vector<std::string> errors;
};

/**
 * @brief Reads every .json file of a directory as a level, in the order of
 * the file names. Files that are too large or invalid are skipped and
 * reported; this never throws.
 * @param directory The directory holding the level files.
 * @return The levels and the errors.
 **/
[[nodiscard]] LevelSet loadLevels(const std::filesystem::path &directory);

} // namespace server

#endif // SERVER_LEVEL_HPP
