/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** Level
*/

#include "Level.hpp"
#include "common/Playfield.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace server {

namespace {

/** @brief The JSON library's document type. */
using Json = nlohmann::json;

/** @brief What is wrong in a level file, caught by parseLevel(). */
class LevelError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

/** @brief The largest time between two enemies of a wave, in seconds. */
constexpr float MaxSpacing = 10.0F;
/** @brief The largest frequency of a movement pattern, in hertz. */
constexpr float MaxFrequency = 10.0F;
/** @brief The longest delay before an enemy dives, in seconds. */
constexpr float MaxDiveDelay = 60.0F;
/** @brief The largest speed of a dive, in playfield units per second. */
constexpr float MaxDiveSpeed = 5000.0F;

/** @brief The enemy names a wave may use. */
constexpr std::array<std::pair<std::string_view, rtype::EnemyType>, 6>
    EnemyNames{{
        {"basic", rtype::EnemyType::Basic},
        {"wave", rtype::EnemyType::Wave},
        {"shooter", rtype::EnemyType::Shooter},
        {"small", rtype::EnemyType::Small},
        {"medium", rtype::EnemyType::Medium},
        {"large", rtype::EnemyType::Large},
    }};

/** @brief The boss names a level may use. */
constexpr std::array<std::pair<std::string_view, rtype::EnemyType>, 1>
    BossNames{{
        {"warship", rtype::EnemyType::Boss},
    }};

/** @brief The movement pattern names a wave may use. */
constexpr std::array<std::pair<std::string_view, rtype::MovementKind>, 4>
    PatternNames{{
        {"straight", rtype::MovementKind::Straight},
        {"sine", rtype::MovementKind::Sine},
        {"zigzag", rtype::MovementKind::Zigzag},
        {"dive", rtype::MovementKind::Dive},
    }};

/**
 * @brief Rejects the keys of an object that are not allowed.
 * @param object The JSON object.
 * @param allowed Its allowed keys.
 * @param where Where the object is in the file, for the error message.
 * @throws LevelError If the value is not an object or has another key.
 **/
void checkKeys(const Json &object,
               std::initializer_list<std::string_view> allowed,
               const std::string &where)
{
    if (!object.is_object()) {
        throw LevelError(where + ": must be an object");
    }
    for (const auto &item : object.items()) {
        if (std::ranges::find(allowed, item.key()) == allowed.end()) {
            throw LevelError(where + ": unknown key \"" + item.key() + "\"");
        }
    }
}

/**
 * @brief Reads a number between min and max.
 * @param object The JSON object holding it.
 * @param key Its key.
 * @param min The smallest value allowed.
 * @param max The largest value allowed.
 * @param fallback Its value when the key is missing, or nothing if it is
 * required.
 * @param where Where the object is in the file, for the error message.
 * @return The number.
 * @throws LevelError If it is missing, not a number or out of range.
 **/
float number(const Json &object, const char *key, float min, float max,
             std::optional<float> fallback, const std::string &where)
{
    const auto it = object.find(key);
    if (it == object.end()) {
        if (!fallback) {
            throw LevelError(where + ": missing \"" + key + "\"");
        }
        return *fallback;
    }
    if (!it->is_number()) {
        throw LevelError(where + "." + key + ": must be a number");
    }
    const auto value = it->get<double>();
    if (!std::isfinite(value) || value < min || value > max) {
        std::ostringstream message;
        message << where << "." << key << ": must be from " << min << " to "
                << max;
        throw LevelError(message.str());
    }
    return static_cast<float>(value);
}

/**
 * @brief Reads a name from a list of allowed names.
 * @param object The JSON object holding it.
 * @param key Its key.
 * @param names The allowed names and their values.
 * @param where Where the object is in the file, for the error message.
 * @return The value of the name.
 * @throws LevelError If it is missing, not a string or not in the list.
 **/
template <typename T, std::size_t N>
T named(const Json &object, const char *key,
        const std::array<std::pair<std::string_view, T>, N> &names,
        const std::string &where)
{
    const auto it = object.find(key);
    if (it == object.end()) {
        throw LevelError(where + ": missing \"" + key + "\"");
    }
    if (it->is_string()) {
        const auto &value = it->get_ref<const std::string &>();
        for (const auto &[name, result] : names) {
            if (value == name) {
                return result;
            }
        }
    }
    std::string list;
    for (const auto &[name, result] : names) {
        list += (list.empty() ? "" : ", ") + std::string{name};
    }
    throw LevelError(where + "." + key + ": must be one of " + list);
}

/**
 * @brief Reads the optional path of a wave.
 * @param pattern The JSON object of the path.
 * @param where Where it is in the file, for the error message.
 * @return The movement pattern.
 * @throws LevelError If it is not a valid path.
 **/
rtype::MovementPattern readPattern(const Json &pattern,
                                   const std::string &where)
{
    checkKeys(pattern,
              {"kind", "amplitude", "frequency", "diveDelay", "diveSpeed"},
              where);
    return {
        .kind = named(pattern, "kind", PatternNames, where),
        .amplitude = number(pattern, "amplitude", 0.0F, rtype::PlayfieldHeight,
                            0.0F, where),
        .frequency =
            number(pattern, "frequency", 0.0F, MaxFrequency, 0.0F, where),
        .diveDelay = engine::Duration{number(pattern, "diveDelay", 0.0F,
                                             MaxDiveDelay, 0.0F, where)},
        .diveSpeed =
            number(pattern, "diveSpeed", 0.0F, MaxDiveSpeed, 0.0F, where),
    };
}

/**
 * @brief Reads one wave.
 * @param wave The JSON object of the wave.
 * @param where Where it is in the file, for the error message.
 * @return The wave.
 * @throws LevelError If it is not a valid wave.
 **/
Wave readWave(const Json &wave, const std::string &where)
{
    checkKeys(wave,
              {"time", "enemy", "count", "spacing", "stepY", "fromCenter", "y",
               "pattern"},
              where);
    const auto fromCenter = wave.find("fromCenter");
    if (fromCenter != wave.end() && !fromCenter->is_boolean()) {
        throw LevelError(where + ".fromCenter: must be true or false");
    }
    const auto count = wave.find("count");
    if (count != wave.end() &&
        (!count->is_number_integer() || count->get<std::int64_t>() < 1 ||
         count->get<std::int64_t>() >
             static_cast<std::int64_t>(MaxWaveCount))) {
        throw LevelError(where + ".count: must be a whole number from 1 to " +
                         std::to_string(MaxWaveCount));
    }
    Wave result{
        .time = engine::Duration{number(wave, "time", 0.0F, MaxWaveTime,
                                        std::nullopt, where)},
        .formation =
            {
                .type = named(wave, "enemy", EnemyNames, where),
                .count = count != wave.end() ? count->get<std::size_t>() : 1,
                .spacing = engine::Duration{number(wave, "spacing", 0.0F,
                                                   MaxSpacing, 0.0F, where)},
                .stepY = number(wave, "stepY", 0.0F, rtype::PlayfieldHeight,
                                0.0F, where),
                .fromCenter =
                    fromCenter != wave.end() && fromCenter->get<bool>(),
            },
    };
    if (wave.contains("y")) {
        result.centerY = number(wave, "y", 0.0F, rtype::PlayfieldHeight,
                                std::nullopt, where);
    }
    if (const auto pattern = wave.find("pattern"); pattern != wave.end()) {
        result.formation.pattern = readPattern(*pattern, where + ".pattern");
    }
    return result;
}

/**
 * @brief Reads a level from its parsed JSON.
 * @param root The JSON document.
 * @return The level.
 * @throws LevelError If it is not a valid level.
 **/
Level readLevel(const Json &root)
{
    checkKeys(root, {"name", "waves", "boss"}, "level");
    const auto name = root.find("name");
    if (name == root.end() || !name->is_string() ||
        name->get_ref<const std::string &>().empty() ||
        name->get_ref<const std::string &>().size() > MaxLevelNameLength) {
        throw LevelError("level.name: must be a text of 1 to " +
                         std::to_string(MaxLevelNameLength) + " characters");
    }
    const auto waves = root.find("waves");
    if (waves == root.end() || !waves->is_array() || waves->empty() ||
        waves->size() > MaxWaves) {
        throw LevelError("level.waves: must be a list of 1 to " +
                         std::to_string(MaxWaves) + " waves");
    }
    Level level{.name = name->get<std::string>()};
    level.waves.reserve(waves->size());
    for (std::size_t i = 0; i < waves->size(); ++i) {
        level.waves.push_back(
            readWave(waves->at(i), "waves[" + std::to_string(i) + "]"));
    }
    std::ranges::stable_sort(level.waves, {}, &Wave::time);
    if (root.contains("boss")) {
        level.boss = named(root, "boss", BossNames, "level");
    }
    return level;
}

} // namespace

std::optional<Level> parseLevel(std::string_view text, std::string &error)
{
    try {
        return readLevel(Json::parse(text));
    } catch (const Json::exception &exception) {
        error = exception.what();
    } catch (const LevelError &exception) {
        error = exception.what();
    }
    return std::nullopt;
}

LevelSet loadLevels(const std::filesystem::path &directory)
{
    LevelSet set;
    std::error_code code;
    std::vector<std::filesystem::path> files;
    for (std::filesystem::directory_iterator it(directory, code), end;
         !code && it != end; it.increment(code)) {
        if (it->path().extension() == ".json" && it->is_regular_file(code)) {
            files.push_back(it->path());
        }
    }
    if (code) {
        set.errors.push_back(directory.string() + ": " + code.message());
        return set;
    }
    std::ranges::sort(files);
    for (const std::filesystem::path &file : files) {
        const std::string name = file.filename().string();
        const std::uintmax_t size = std::filesystem::file_size(file, code);
        if (code || size > MaxLevelFileSize) {
            set.errors.push_back(name + ": cannot be read or larger than " +
                                 std::to_string(MaxLevelFileSize) + " bytes");
            continue;
        }
        std::ifstream in(file, std::ios::binary);
        const std::string text{std::istreambuf_iterator<char>(in),
                               std::istreambuf_iterator<char>()};
        std::string error;
        if (auto level = parseLevel(text, error)) {
            set.levels.push_back(std::move(*level));
        } else {
            set.errors.push_back(name);
            set.errors.back().append(": ").append(error);
        }
    }
    return set;
}

} // namespace server
