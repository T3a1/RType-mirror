/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** EnemyCatalog
*/

#include "EnemyCatalog.hpp"

#include <algorithm>
#include <array>
#include <utility>

namespace server {

namespace {

/** @brief The names of the built-in kinds, in the order of rtype::EnemyType.
 */
constexpr std::array<std::string_view, ENEMY_DEFS.size()> BuiltinNames{
    "basic", "wave", "shooter", "small", "medium", "large", "warship", "hive",
};

/**
 * @brief The catalog of the server.
 * @return It, built on first use.
 **/
EnemyCatalog &catalog()
{
    static EnemyCatalog instance;
    return instance;
}

} // namespace

EnemyCatalog::EnemyCatalog()
{
    m_kinds.reserve(ENEMY_DEFS.size());
    for (std::size_t i = 0; i < ENEMY_DEFS.size(); ++i) {
        m_kinds.push_back({
            .name = std::string{BuiltinNames.at(i)},
            .behavior = static_cast<rtype::EnemyType>(i),
            .def = ENEMY_DEFS.at(i),
        });
    }
}

std::optional<EnemyKindId> EnemyCatalog::find(std::string_view name) const
{
    const auto it = std::ranges::find(m_kinds, name, &EnemyKind::name);
    if (it == m_kinds.end()) {
        return std::nullopt;
    }
    return static_cast<EnemyKindId>(it - m_kinds.begin());
}

const EnemyKind &EnemyCatalog::at(EnemyKindId id) const
{
    return id < m_kinds.size() ? m_kinds.at(id) : m_kinds.front();
}

void EnemyCatalog::set(EnemyKindId id, const EnemyDef &def)
{
    if (id < m_kinds.size()) {
        m_kinds.at(id).def = def;
    }
}

std::optional<EnemyKindId> EnemyCatalog::add(EnemyKind kind)
{
    if (m_kinds.size() >= MaxEnemyKinds || find(kind.name)) {
        return std::nullopt;
    }
    m_kinds.push_back(std::move(kind));
    return static_cast<EnemyKindId>(m_kinds.size() - 1);
}

const EnemyDef &enemyDef(rtype::EnemyType type)
{
    return catalog().at(builtinKind(type)).def;
}

const EnemyDef &enemyDef(const rtype::Enemy &enemy)
{
    // Enemies built without a kind (kind 0) still get the stats of their type.
    if (enemy.kind < ENEMY_DEFS.size()) {
        return enemyDef(enemy.type);
    }
    return catalog().at(enemy.kind).def;
}

const EnemyCatalog &enemyCatalog() { return catalog(); }

void setEnemyCatalog(EnemyCatalog catalog)
{
    server::catalog() = std::move(catalog);
}

} // namespace server
