/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EnemyCatalog
*/

#ifndef SERVER_ENEMYCATALOG_HPP
#define SERVER_ENEMYCATALOG_HPP

#include "EnemyDefs.hpp"
#include "common/components/Enemy.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace server {

/** @brief The index of an enemy kind in the EnemyCatalog. The built-in kinds
 * come first, in the order of rtype::EnemyType. */
using EnemyKindId = std::uint16_t;

/** @brief The most enemy kinds a catalog can hold, built-in ones included. */
inline constexpr std::size_t MaxEnemyKinds = 256;

/**
 * @struct EnemyKind
 * @brief One kind of enemy the levels can call by its name.
 * @param name Its name in the level files, such as "small".
 * @param behavior The built-in type whose code moves and arms it: a boss is
 * moved by bossSystem(), the others by their pattern and Weapon.
 * @param def Its stats, including the sprite the clients draw for it.
 **/
struct EnemyKind {
    std::string name;
    rtype::EnemyType behavior = rtype::EnemyType::Basic;
    EnemyDef def;
};

/**
 * @class EnemyCatalog
 * @brief Every kind of enemy of the game: the built-in ones, with the stats
 * of ENEMY_DEFS or of server/enemies.json, then the ones the file adds.
 **/
class EnemyCatalog {
  public:
    /** @brief Builds the catalog of the built-in kinds, with their default
     * stats. */
    EnemyCatalog();

    /**
     * @brief Finds a kind by its name.
     * @param name The name used in the level files.
     * @return Its id, or nothing if no kind has that name.
     **/
    [[nodiscard]] std::optional<EnemyKindId> find(std::string_view name) const;

    /**
     * @brief Returns a kind; an unknown id gives the first built-in kind.
     * @param id The id of the kind.
     * @return The kind.
     **/
    [[nodiscard]] const EnemyKind &at(EnemyKindId id) const;

    /** @brief Returns every kind, built-in ones first. */
    [[nodiscard]] const std::vector<EnemyKind> &kinds() const
    {
        return m_kinds;
    }

    /**
     * @brief Replaces the stats of a kind.
     * @param id The id of the kind.
     * @param def Its new stats.
     **/
    void set(EnemyKindId id, const EnemyDef &def);

    /**
     * @brief Adds a kind at the end of the catalog.
     * @param kind The new kind; its name must not be taken.
     * @return Its id, or nothing if the catalog is full or the name taken.
     **/
    std::optional<EnemyKindId> add(EnemyKind kind);

  private:
    std::vector<EnemyKind> m_kinds;
};

/**
 * @brief Returns the id of a built-in enemy type in every catalog.
 * @param type The built-in type.
 * @return Its id.
 **/
constexpr EnemyKindId builtinKind(rtype::EnemyType type)
{
    return static_cast<EnemyKindId>(type);
}

/**
 * @brief Returns the catalog of the server: the built-in kinds until
 * setEnemyCatalog() is called.
 * @return The catalog, shared by every room.
 **/
[[nodiscard]] const EnemyCatalog &enemyCatalog();

/**
 * @brief Replaces the catalog of the server. Call it once at startup, before
 * any room runs: the rooms read the catalog from their own threads without
 * locking.
 * @param catalog The new catalog.
 **/
void setEnemyCatalog(EnemyCatalog catalog);

} // namespace server

#endif // SERVER_ENEMYCATALOG_HPP
