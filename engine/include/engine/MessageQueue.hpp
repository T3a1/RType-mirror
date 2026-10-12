/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** MessageQueue
*/

#ifndef ENGINE_MESSAGEQUEUE_HPP
#define ENGINE_MESSAGEQUEUE_HPP

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <limits>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace engine {

template <typename T> class MessageQueue {
  public:
    explicit MessageQueue(
        std::size_t capacity = std::numeric_limits<std::size_t>::max())
        : m_capacity(capacity)
    {
    }

    bool push(T &&message)
    {
        {
            const std::scoped_lock lock(m_mutex);
            if (m_closed || m_messages.size() >= m_capacity) {
                return false;
            }
            m_messages.push_back(std::move(message));
        }
        m_ready.notify_one();
        return true;
    }

    bool push(const T &message)
    {
        T copy = message;
        return push(std::move(copy));
    }

    std::vector<T> drain()
    {
        std::vector<T> out;
        const std::scoped_lock lock(m_mutex);
        out.reserve(m_messages.size());
        for (T &message : m_messages) {
            out.push_back(std::move(message));
        }
        m_messages.clear();
        return out;
    }

    std::optional<T> waitPop()
    {
        std::unique_lock lock(m_mutex);
        m_ready.wait(lock, [this] { return m_closed || !m_messages.empty(); });
        if (m_messages.empty()) {
            return std::nullopt;
        }
        T message = std::move(m_messages.front());
        m_messages.pop_front();
        return message;
    }

    void close()
    {
        {
            const std::scoped_lock lock(m_mutex);
            m_closed = true;
        }
        m_ready.notify_all();
    }

    [[nodiscard]] bool closed() const
    {
        const std::scoped_lock lock(m_mutex);
        return m_closed;
    }

    [[nodiscard]] std::size_t size() const
    {
        const std::scoped_lock lock(m_mutex);
        return m_messages.size();
    }

  private:
    mutable std::mutex m_mutex;
    std::condition_variable m_ready;
    std::deque<T> m_messages;
    std::size_t m_capacity;
    bool m_closed = false;
};

} // namespace engine

#endif // ENGINE_MESSAGEQUEUE_HPP
