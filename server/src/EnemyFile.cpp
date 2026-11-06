/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EnemyFile
*/

#include "EnemyCatalog.hpp"
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
#include <type_traits>
#include <utility>

namespace server {

namespace {

/** @brief The JSON library's document type. */
using Json = nlohmann::json;

/** @brief Enemy files larger than this are not read, in bytes. */
constexpr std::uintmax_t MaxEnemiesFileSize = std::uintmax_t{256} * 1024U;
/** @brief The most health an enemy can have: snapshots send it on 16 bits,
 * and the hard difficulty raises it by half. */
constexpr double MaxEnemyHealth = 40000.0;
/** @brief The longest name of an enemy kind, in characters. */
constexpr std::size_t MaxEnemyNameLength = 32;

/** @brief What is wrong in an enemies file, caught by parseEnemies(). */
class EnemiesError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

/** @brief The movement pattern names an enemy may use. */
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
 * @throws EnemiesError If the value is not an object or has another key.
 **/
void checkKeys(const Json &object,
               std::initializer_list<std::string_view> allowed,
               const std::string &where)
{
    if (!object.is_object()) {
        throw EnemiesError(where + ": must be an object");
    }
    for (const auto &item : object.items()) {
        if (std::ranges::find(allowed, item.key()) == allowed.end()) {
            throw EnemiesError(where + ": unknown key \"" + item.key() + "\"");
        }
    }
}

/**
 * @brief Overwrites a value with a number of the file, if it is there.
 * @param object The JSON object.
 * @param key The key of the number.
 * @param min The smallest value allowed.
 * @param max The largest value allowed.
 * @param where Where the object is in the file, for the error message.
 * @param value The value to overwrite.
 * @throws EnemiesError If it is not a number or out of range.
 **/
template <typename T>
void read(const Json &object, const char *key, double min, double max,
          const std::string &where, T &value)
{
    const auto it = object.find(key);
    if (it == object.end()) {
        return;
    }
    const bool whole = std::is_integral_v<T>;
    if (!it->is_number() || (whole && !it->is_number_integer())) {
        throw EnemiesError(where + "." + key + ": must be a " +
                           (whole ? "whole number" : "number"));
    }
    const auto number = it->get<double>();
    if (!std::isfinite(number) || number < min || number > max) {
        std::ostringstream message;
        message << where << "." << key << ": must be from " << min << " to "
                << max;
        throw EnemiesError(message.str());
    }
    value = static_cast<T>(number);
}

/**
 * @brief Overwrites a duration with a number of seconds, if it is there.
 * @param object The JSON object.
 * @param key The key of the number.
 * @param max The largest value allowed, in seconds.
 * @param where Where the object is in the file, for the error message.
 * @param value The duration to overwrite.
 * @throws EnemiesError If it is not a number or out of range.
 **/
void readSeconds(const Json &object, const char *key, double max,
                 const std::string &where, engine::Duration &value)
{
    float seconds = value.count();
    read(object, key, 0.0, max, where, seconds);
    value = engine::Duration{seconds};
}

/**
 * @brief Finds a built-in kind by the name given under a key.
 * @param object The JSON object.
 * @param key The key of the name.
 * @param catalog The catalog being built.
 * @param where Where the object is in the file, for the error message.
 * @return The built-in kind.
 * @throws EnemiesError If the name is not a string or not a built-in kind.
 **/
const EnemyKind &builtin(const Json &object, const char *key,
                         const EnemyCatalog &catalog, const std::string &where)
{
    const auto &value = object.at(key);
    const auto id = value.is_string()
                        ? catalog.find(value.get_ref<const std::string &>())
                        : std::nullopt;
    if (!id || *id >= ENEMY_DEFS.size()) {
        std::string list;
        for (std::size_t i = 0; i < ENEMY_DEFS.size(); ++i) {
            list += (i == 0 ? "" : ", ") + catalog.kinds().at(i).name;
        }
        throw EnemiesError(where + "." + key + ": must be one of " + list);
    }
    return catalog.at(*id);
}

/**
 * @brief Overwrites a movement pattern with the one of the file, if it is
 * there.
 * @param object The JSON object of the enemy.
 * @param where Where it is in the file, for the error message.
 * @param pattern The pattern to overwrite.
 * @throws EnemiesError If it is not a valid pattern.
 **/
void readPattern(const Json &object, const std::string &where,
                 rtype::MovementPattern &pattern)
{
    const auto it = object.find("pattern");
    if (it == object.end()) {
        return;
    }
    const std::string at = where + ".pattern";
    checkKeys(*it, {"kind", "amplitude", "frequency", "diveDelay", "diveSpeed"},
              at);
    const auto kind = it->find("kind");
    const std::string name = kind != it->end() && kind->is_string()
                                 ? kind->get<std::string>()
                                 : std::string{};
    bool known = false;
    for (const auto &[patternName, value] : PatternNames) {
        if (name == patternName) {
            pattern = rtype::MovementPattern{.kind = value};
            known = true;
        }
    }
    if (!known) {
        throw EnemiesError(
            at + ".kind: must be one of straight, sine, zigzag, dive");
    }
    read(*it, "amplitude", 0.0, rtype::PlayfieldHeight, at, pattern.amplitude);
    read(*it, "frequency", 0.0, 10.0, at, pattern.frequency);
    readSeconds(*it, "diveDelay", 60.0, at, pattern.diveDelay);
    read(*it, "diveSpeed", 0.0, 5000.0, at, pattern.diveSpeed);
}

/**
 * @brief Overwrites the stats of an enemy with the keys of its object.
 * @param object The JSON object of the enemy.
 * @param catalog The catalog being built, for the "look" key.
 * @param where Where it is in the file, for the error message.
 * @param def The stats to overwrite.
 * @throws EnemiesError If a key is not valid.
 **/
void readStats(const Json &object, const EnemyCatalog &catalog,
               const std::string &where, EnemyDef &def)
{
    read(object, "health", 1.0, MaxEnemyHealth, where, def.health);
    read(object, "speed", 0.0, 3000.0, where, def.speed);
    read(object, "width", 1.0, rtype::PlayfieldHeight, where, def.size.x);
    read(object, "height", 1.0, rtype::PlayfieldHeight, where, def.size.y);
    read(object, "score", 0.0, 1000000.0, where, def.score);
    readSeconds(object, "fireInterval", 60.0, where, def.fireInterval);
    readPattern(object, where, def.pattern);
    if (const auto drops = object.find("dropsPowerUp"); drops != object.end()) {
        if (!drops->is_boolean()) {
            throw EnemiesError(where + ".dropsPowerUp: must be true or false");
        }
        def.dropsPowerUp = drops->get<bool>();
    }
    if (object.contains("look")) {
        def.entityType = builtin(object, "look", catalog, where).def.entityType;
    }
}

/**
 * @brief Reads one enemy: new stats for a built-in kind, or a new kind based
 * on a built-in one.
 * @param name Its name.
 * @param object Its JSON object.
 * @param catalog The catalog it goes to.
 * @throws EnemiesError If it is not valid.
 **/
void readEnemy(const std::string &name, const Json &object,
               EnemyCatalog &catalog)
{
    const std::string where = "enemies." + name;
    const auto id = catalog.find(name);
    if (id && *id < ENEMY_DEFS.size()) {
        checkKeys(object,
                  {"look", "health", "speed", "width", "height", "score",
                   "fireInterval", "pattern", "dropsPowerUp"},
                  where);
        EnemyDef def = catalog.at(*id).def;
        readStats(object, catalog, where, def);
        catalog.set(*id, def);
        return;
    }
    if (name.empty() || name.size() > MaxEnemyNameLength) {
        throw EnemiesError(where + ": a name has 1 to " +
                           std::to_string(MaxEnemyNameLength) + " characters");
    }
    checkKeys(object,
              {"base", "look", "health", "speed", "width", "height", "score",
               "fireInterval", "pattern", "dropsPowerUp"},
              where);
    if (!object.contains("base")) {
        throw EnemiesError(where + ": missing \"base\", the built-in enemy it "
                                   "starts from");
    }
    const EnemyKind &base = builtin(object, "base", catalog, where);
    EnemyKind kind{.name = name, .behavior = base.behavior, .def = base.def};
    readStats(object, catalog, where, kind.def);
    if (!catalog.add(std::move(kind))) {
        throw EnemiesError(where + ": at most " +
                           std::to_string(MaxEnemyKinds) + " kinds of enemies");
    }
}

} // namespace

std::optional<EnemyCatalog> parseEnemies(std::string_view text,
                                         std::string &error)
{
    try {
        const Json root = Json::parse(text);
        checkKeys(root, {"enemies"}, "file");
        EnemyCatalog catalog;
        const auto enemies = root.find("enemies");
        if (enemies == root.end()) {
            return catalog;
        }
        if (!enemies->is_object()) {
            throw EnemiesError("enemies: must be an object of named enemies");
        }
        for (const auto &item : enemies->items()) {
            readEnemy(item.key(), item.value(), catalog);
        }
        return catalog;
    } catch (const Json::exception &exception) {
        error = exception.what();
    } catch (const EnemiesError &exception) {
        error = exception.what();
    }
    return std::nullopt;
}

EnemyCatalog loadEnemies(const std::filesystem::path &file, std::string &error)
{
    std::error_code code;
    const std::uintmax_t size = std::filesystem::file_size(file, code);
    if (code || size > MaxEnemiesFileSize) {
        error = file.string() + ": missing or larger than " +
                std::to_string(MaxEnemiesFileSize) + " bytes";
        return {};
    }
    std::ifstream in(file, std::ios::binary);
    const std::string text{std::istreambuf_iterator<char>(in),
                           std::istreambuf_iterator<char>()};
    std::string message;
    if (auto catalog = parseEnemies(text, message)) {
        return std::move(*catalog);
    }
    error = file.filename().string();
    error.append(": ").append(message);
    return {};
}

} // namespace server
