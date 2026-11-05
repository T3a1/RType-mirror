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

/**
 * @class MessageQueue
 * @brief A thread-safe queue of messages between two threads, with a capacity
 * so that a flood can never use unbounded memory.
 * @tparam T The type of the messages.
 **/
template <typename T> class MessageQueue {
  public:
    /**
     * @brief Creates an empty queue.
     * @param capacity How many messages it may hold; no limit by default.
     **/
    explicit MessageQueue(
        std::size_t capacity = std::numeric_limits<std::size_t>::max())
        : m_capacity(capacity)
    {
    }

    /**
     * @brief Adds a message at the end and wakes a waiting thread.
     * @param message The message, moved from if it is accepted.
     * @return `true` if it was added, `false` if the queue is full or closed.
     **/
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

    /**
     * @brief Adds a copy of a message at the end and wakes a waiting thread.
     * @param message The message.
     * @return `true` if it was added, `false` if the queue is full or closed.
     **/
    bool push(const T &message)
    {
        T copy = message;
        return push(std::move(copy));
    }

    /**
     * @brief Takes all the messages at once, without waiting.
     * @return The messages, oldest first; empty if there are none.
     **/
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

    /**
     * @brief Waits for a message and takes it.
     * @return The oldest message, none once the queue is closed and empty.
     **/
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

    /**
     * @brief Refuses new messages and wakes the waiting threads. The messages
     * already there can still be taken.
     **/
    void close()
    {
        {
            const std::scoped_lock lock(m_mutex);
            m_closed = true;
        }
        m_ready.notify_all();
    }

    /**
     * @brief Tells whether close() was called.
     * @return `true` if the queue is closed.
     **/
    [[nodiscard]] bool closed() const
    {
        const std::scoped_lock lock(m_mutex);
        return m_closed;
    }

    /**
     * @brief Returns how many messages are waiting.
     * @return The number of messages.
     **/
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
