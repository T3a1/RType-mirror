/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** InstanceRunner
*/

#ifndef ENGINE_INSTANCERUNNER_HPP
#define ENGINE_INSTANCERUNNER_HPP

#include "engine/IInstance.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

namespace engine {

/**
 * @class InstanceRunner
 * @brief Runs one IInstance on its own thread at a fixed time step, so that
 * a slow instance only slows itself, and catches its exceptions, so that a
 * failing instance only ends itself.
 **/
class InstanceRunner {
  public:
    using Step = std::chrono::steady_clock::duration;

    /**
     * @brief Starts the thread.
     * @param instance The instance to run.
     * @param step The fixed time step between two ticks.
     **/
    InstanceRunner(std::unique_ptr<IInstance> instance, Step step);

    /**
     * @brief Asks the thread to stop and waits for it.
     **/
    ~InstanceRunner();

    InstanceRunner(const InstanceRunner &) = delete;
    InstanceRunner &operator=(const InstanceRunner &) = delete;
    InstanceRunner(InstanceRunner &&) = delete;
    InstanceRunner &operator=(InstanceRunner &&) = delete;

    /**
     * @brief Asks the thread to stop after the current tick, without
     * waiting for it.
     **/
    void requestStop() { m_stop = true; }

    /**
     * @brief Returns whether the thread is over, because it was asked to
     * stop or because the instance failed. Destroying the runner then does
     * not block.
     * @return True once the thread is over.
     **/
    [[nodiscard]] bool done() const { return m_done; }

    /**
     * @brief Returns whether the instance accepts new members.
     * @return False once the thread is over.
     **/
    [[nodiscard]] bool joinable() const;

    /**
     * @brief Returns the status of the instance.
     * @return IInstance::status().
     **/
    [[nodiscard]] std::uint8_t status() const { return m_instance->status(); }

  private:
    void run();

    std::unique_ptr<IInstance> m_instance;
    Step m_step;
    std::atomic<bool> m_stop{false};
    std::atomic<bool> m_done{false};
    std::thread m_thread;
};

} // namespace engine

#endif // ENGINE_INSTANCERUNNER_HPP
