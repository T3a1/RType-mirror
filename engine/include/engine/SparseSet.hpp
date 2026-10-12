/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** SparseSet
*/

#ifndef ENGINE_SPARSESET_HPP
#define ENGINE_SPARSESET_HPP

#include "engine/Entity.hpp"

#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

namespace engine {

class ISparseSet {
  public:
    ISparseSet() = default;
    virtual ~ISparseSet() = default;

    ISparseSet(const ISparseSet &) = delete;
    ISparseSet &operator=(const ISparseSet &) = delete;
    ISparseSet(ISparseSet &&) = delete;
    ISparseSet &operator=(ISparseSet &&) = delete;

    virtual void remove(Entity entity) = 0;
    [[nodiscard]] virtual bool contains(Entity entity) const = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
};

template <typename C> class SparseSet final : public ISparseSet {
  public:
    template <typename... Args> C &emplace(Entity entity, Args &&...args)
    {
        if (contains(entity)) {
            C &component = m_data[m_sparse[entity.id]];
            component = C{std::forward<Args>(args)...};
            return component;
        }
        if (entity.id >= m_sparse.size()) {
            m_sparse.resize(static_cast<std::size_t>(entity.id) + 1, NONE);
        }
        m_sparse[entity.id] = m_dense.size();
        m_dense.push_back(entity);
        m_data.push_back(C{std::forward<Args>(args)...});
        return m_data.back();
    }

    void remove(Entity entity) override
    {
        if (!contains(entity)) {
            return;
        }
        const std::size_t hole = m_sparse[entity.id];
        const std::size_t last = m_dense.size() - 1;

        if (hole != last) {
            m_dense[hole] = m_dense[last];
            m_data[hole] = std::move(m_data[last]);
            m_sparse[m_dense[hole].id] = hole;
        }
        m_dense.pop_back();
        m_data.pop_back();
        m_sparse[entity.id] = NONE;
    }

    [[nodiscard]] bool contains(Entity entity) const override
    {
        return entity.id < m_sparse.size() && m_sparse[entity.id] != NONE &&
               m_dense[m_sparse[entity.id]] == entity;
    }

    [[nodiscard]] std::size_t size() const override { return m_dense.size(); }

    C &get(Entity entity)
    {
        assert(contains(entity));
        return m_data[m_sparse[entity.id]];
    }

    C *tryGet(Entity entity)
    {
        return contains(entity) ? &m_data[m_sparse[entity.id]] : nullptr;
    }

    [[nodiscard]] Entity entityAt(std::size_t position) const
    {
        return m_dense[position];
    }

    C &dataAt(std::size_t position) { return m_data[position]; }

  private:
    static constexpr std::size_t NONE = std::numeric_limits<std::size_t>::max();

    std::vector<std::size_t> m_sparse;
    std::vector<Entity> m_dense;
    std::vector<C> m_data;
};

} // namespace engine

#endif // ENGINE_SPARSESET_HPP
