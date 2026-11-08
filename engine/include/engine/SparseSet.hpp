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

/**
 * @class ISparseSet
 * @brief The part of a component storage that does not depend on the component
 * type, so that the registry can keep all its storages together.
 **/
class ISparseSet {
  public:
    ISparseSet() = default;
    virtual ~ISparseSet() = default;

    ISparseSet(const ISparseSet &) = delete;
    ISparseSet &operator=(const ISparseSet &) = delete;
    ISparseSet(ISparseSet &&) = delete;
    ISparseSet &operator=(ISparseSet &&) = delete;

    /**
     * @brief Removes the component of an entity, if it has one.
     * @param entity The entity.
     **/
    virtual void remove(Entity entity) = 0;
    /**
     * @brief Tells whether an entity has a component here.
     * @param entity The entity, compared with its generation.
     * @return `true` if it has one.
     **/
    [[nodiscard]] virtual bool contains(Entity entity) const = 0;
    /**
     * @brief Returns how many components are stored.
     * @return The number of components.
     **/
    [[nodiscard]] virtual std::size_t size() const = 0;
};

/**
 * @class SparseSet
 * @brief The storage of one component type: add, remove and lookup in O(1),
 * and components packed in an array for a fast walk.
 *
 * `m_sparse` gives the position of an entity index in the two packed arrays.
 * Removing moves the last component into the hole, so positions change.
 * @tparam C The component type.
 **/
template <typename C> class SparseSet final : public ISparseSet {
  public:
    /**
     * @brief Adds a component to an entity, replacing the one it may already
     *have.
     * @param entity The entity.
     * @param args What to initialize the component with.
     * @return The component. Adding or removing may invalidate it.
     **/
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

    /**
     * @brief Returns the component of an entity that must have one (checked
     *with an assert only).
     * @param entity The entity.
     * @return The component.
     **/
    C &get(Entity entity)
    {
        assert(contains(entity));
        return m_data[m_sparse[entity.id]];
    }

    /**
     * @brief Returns the component of an entity that may not have one.
     * @param entity The entity.
     * @return The component, nullptr if it has none.
     **/
    C *tryGet(Entity entity)
    {
        return contains(entity) ? &m_data[m_sparse[entity.id]] : nullptr;
    }

    /**
     * @brief Returns the entity stored at a position, to walk the storage.
     * @param position The position, below size().
     * @return The entity.
     **/
    [[nodiscard]] Entity entityAt(std::size_t position) const
    {
        return m_dense[position];
    }

    /**
     * @brief Returns the component stored at a position, to walk the storage.
     * @param position The position, below size().
     * @return The component of entityAt(position).
     **/
    C &dataAt(std::size_t position) { return m_data[position]; }

  private:
    static constexpr std::size_t NONE = std::numeric_limits<std::size_t>::max();

    std::vector<std::size_t> m_sparse;
    std::vector<Entity> m_dense;
    std::vector<C> m_data;
};

} // namespace engine

#endif // ENGINE_SPARSESET_HPP
