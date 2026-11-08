/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** InstanceManager
*/

#ifndef ENGINE_INSTANCEMANAGER_HPP
#define ENGINE_INSTANCEMANAGER_HPP

#include "engine/IInstance.hpp"
#include "engine/InstanceRunner.hpp"
#include "engine/Logger.hpp"
#include "engine/MessageQueue.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace engine {

/**
 * @brief The identifier of an instance, from 1.
 **/
using InstanceId = std::uint32_t;

/**
 * @brief Asks InstanceManager::join() for any instance with room, a new one if
 *none.
 **/
inline constexpr InstanceId AnyInstance = 0;

/**
 * @struct InstanceLimits
 * @brief The limits of an InstanceManager.
 * @param maxInstances How many instances may run at the same time.
 * @param maxMembers How many members one instance accepts.
 * @param inboundCapacity The capacity of the queue to each instance.
 **/
struct InstanceLimits {
    std::size_t maxInstances = 1;
    std::size_t maxMembers = 1;
    std::size_t inboundCapacity = 1024;
};

/**
 * @struct InstanceInfo
 * @brief What is known about a running instance.
 * @param id The instance.
 * @param members How many members it has.
 * @param status IInstance::status().
 **/
struct InstanceInfo {
    InstanceId id = AnyInstance;
    std::size_t members = 0;
    std::uint8_t status = 0;
};

/**
 * @class InstanceManager
 * @brief Runs several independent instances of a game in one process: creates
 * an instance when a first member joins it, stops it when the last member
 * leaves, and gives each one its own message queues and its own thread.
 *
 * It knows nothing about the game: the game provides the message types and a
 * factory building its IInstance. All the calls must come from one thread
 * (usually the network thread), and none of them waits for an instance.
 *
 * @tparam In The messages sent to an instance.
 * @tparam Out The messages sent by an instance.
 **/
template <typename In, typename Out> class InstanceManager {
  public:
    /**
     * @struct Link
     * @brief The queues between one instance and the rest of the program.
     * @param inbound The messages to the instance.
     * @param outbound The messages from the instance.
     **/
    struct Link {
        /**
         * @brief Creates the two queues.
         * @param inboundCapacity The capacity of the inbound queue.
         **/
        explicit Link(std::size_t inboundCapacity) : inbound(inboundCapacity) {}
        MessageQueue<In> inbound;
        MessageQueue<Out> outbound;
    };

    /**
     * @brief Builds the instance of a game connected to its queues.
     **/
    using Factory = std::function<std::unique_ptr<IInstance>(Link &)>;

    /**
     * @struct Membership
     * @brief What join() gives to a new member.
     * @param id The instance joined.
     * @param link Its queues, valid as long as this pointer is kept.
     **/
    struct Membership {
        InstanceId id = AnyInstance;
        std::shared_ptr<Link> link;
    };

    /**
     * @brief Creates a manager without any instance.
     * @param factory Builds the instances.
     * @param limits The limits.
     * @param step The fixed time step of the instances.
     **/
    InstanceManager(Factory factory, InstanceLimits limits,
                    InstanceRunner::Step step)
        : m_factory(std::move(factory)), m_limits(limits), m_step(step)
    {
    }

    /**
     * @brief Adds a member to an instance, created if needed.
     * @param requested The instance to join, or AnyInstance for the first one
     * with room.
     * @return The membership, or nothing if the instance is full or not
     * joinable, if the maximum of instances is reached, or if the factory
     * failed.
     **/
    [[nodiscard]] std::optional<Membership> join(InstanceId requested);

    /**
     * @brief Adds a member to a new instance.
     * @return The membership, or nothing if the maximum of instances is
     * reached or if the factory failed.
     **/
    [[nodiscard]] std::optional<Membership> joinNew() { return join(freeId()); }

    /**
     * @brief Removes a member. After the last one, the instance stops in the
     * background and its id is free again.
     * @param id The instance; ignored if unknown.
     **/
    void leave(InstanceId id);

    /**
     * @brief Destroys the stopped instances, and returns those that ended by
     * themselves because they failed: their members are gone with them, and
     * must not call leave().
     * @return The ids of the failed instances.
     **/
    [[nodiscard]] std::vector<InstanceId> reap();

    /**
     * @brief Calls a function on the queues of every running instance.
     * @param function Called with the InstanceId and the Link.
     **/
    template <typename Function> void forEachLink(Function function)
    {
        for (auto &[id, entry] : m_instances) {
            function(id, *entry.link);
        }
    }

    /**
     * @brief Lists the running instances.
     * @return One entry per instance, by increasing id.
     **/
    [[nodiscard]] std::vector<InstanceInfo> list() const
    {
        std::vector<InstanceInfo> infos;
        infos.reserve(m_instances.size());
        for (const auto &[id, entry] : m_instances) {
            infos.push_back({
                .id = id,
                .members = entry.members,
                .status = entry.runner->status(),
            });
        }
        return infos;
    }

    /**
     * @brief Stops every instance and waits for them, at shutdown.
     **/
    void stopAll()
    {
        m_instances.clear();
        m_stopping.clear();
    }

    /**
     * @brief Returns the number of running instances.
     * @return The number of instances that have members.
     **/
    [[nodiscard]] std::size_t size() const { return m_instances.size(); }

  private:
    // The runner comes after the link: the thread stops before the queues it
    // uses are destroyed.
    struct Entry {
        std::shared_ptr<Link> link;
        std::unique_ptr<InstanceRunner> runner;
        std::size_t members = 0;
    };

    using EntryIterator = std::map<InstanceId, Entry>::iterator;

    [[nodiscard]] bool hasRoom(const Entry &entry) const
    {
        return entry.members < m_limits.maxMembers && entry.runner->joinable();
    }
    [[nodiscard]] EntryIterator findAny();
    [[nodiscard]] InstanceId freeId() const;
    [[nodiscard]] EntryIterator create(InstanceId id);

    Factory m_factory;
    InstanceLimits m_limits;
    InstanceRunner::Step m_step;
    std::map<InstanceId, Entry> m_instances;
    std::vector<Entry> m_stopping;
};

template <typename In, typename Out>
std::optional<typename InstanceManager<In, Out>::Membership>
InstanceManager<In, Out>::join(InstanceId requested)
{
    auto it =
        requested == AnyInstance ? findAny() : m_instances.find(requested);
    const bool isNew = it == m_instances.end();
    if (isNew) {
        it = create(requested == AnyInstance ? freeId() : requested);
    }
    if (it == m_instances.end()) {
        return std::nullopt;
    }
    if (!hasRoom(it->second)) {
        if (isNew) {
            m_instances.erase(it);
        }
        return std::nullopt;
    }
    ++it->second.members;
    return Membership{.id = it->first, .link = it->second.link};
}

template <typename In, typename Out>
void InstanceManager<In, Out>::leave(InstanceId id)
{
    const auto it = m_instances.find(id);
    if (it == m_instances.end()) {
        return;
    }
    if (--it->second.members == 0) {
        it->second.runner->requestStop();
        m_stopping.push_back(std::move(it->second));
        m_instances.erase(it);
    }
}

template <typename In, typename Out>
std::vector<InstanceId> InstanceManager<In, Out>::reap()
{
    std::erase_if(m_stopping,
                  [](const Entry &entry) { return entry.runner->done(); });
    std::vector<InstanceId> failed;
    for (auto it = m_instances.begin(); it != m_instances.end();) {
        if (it->second.runner->done()) {
            failed.push_back(it->first);
            it = m_instances.erase(it);
        } else {
            ++it;
        }
    }
    return failed;
}

template <typename In, typename Out>
InstanceManager<In, Out>::EntryIterator InstanceManager<In, Out>::findAny()
{
    for (auto it = m_instances.begin(); it != m_instances.end(); ++it) {
        if (hasRoom(it->second)) {
            return it;
        }
    }
    return m_instances.end();
}

template <typename In, typename Out>
InstanceId InstanceManager<In, Out>::freeId() const
{
    InstanceId id = AnyInstance + 1;
    while (m_instances.contains(id)) {
        ++id;
    }
    return id;
}

template <typename In, typename Out>
InstanceManager<In, Out>::EntryIterator
InstanceManager<In, Out>::create(InstanceId id)
{
    if (m_instances.size() >= m_limits.maxInstances) {
        return m_instances.end();
    }
    try {
        auto link = std::make_shared<Link>(m_limits.inboundCapacity);
        auto runner =
            std::make_unique<InstanceRunner>(m_factory(*link), m_step);
        return m_instances
            .emplace(id,
                     Entry{
                         .link = std::move(link),
                         .runner = std::move(runner),
                     })
            .first;
    } catch (const std::exception &error) {
        BB_ERROR("Could not create instance " << id << ": " << error.what());
        return m_instances.end();
    }
}

} // namespace engine

#endif // ENGINE_INSTANCEMANAGER_HPP
