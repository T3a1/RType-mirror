/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** InstanceRunner
*/

#include "engine/InstanceRunner.hpp"
#include "engine/Logger.hpp"

#include <algorithm>
#include <exception>
#include <utility>

namespace engine {

namespace {

/** @brief How far behind an instance may fall: older time is dropped. */
constexpr std::chrono::milliseconds MaxCatchUp{250};

} // namespace

InstanceRunner::InstanceRunner(std::unique_ptr<IInstance> instance, Step step)
    : m_instance(std::move(instance)), m_step(step), m_thread([this] { run(); })
{
}

InstanceRunner::~InstanceRunner()
{
    requestStop();
    m_thread.join();
}

bool InstanceRunner::joinable() const
{
    return !m_done && m_instance->joinable();
}

void InstanceRunner::run()
{
    using Clock = std::chrono::steady_clock;
    Step accumulator{0};
    Clock::time_point previous = Clock::now();
    try {
        while (!m_stop) {
            const Clock::time_point now = Clock::now();
            accumulator =
                std::min<Step>(accumulator + (now - previous), MaxCatchUp);
            previous = now;
            while (accumulator >= m_step) {
                m_instance->tick(Duration{m_step});
                accumulator -= m_step;
            }
            std::this_thread::sleep_until(now + (m_step - accumulator));
        }
        m_instance->onStop();
    } catch (const std::exception &error) {
        BB_ERROR("Instance stopped by an error: " << error.what());
    } catch (...) {
        BB_ERROR("Instance stopped by an unknown error");
    }
    m_done = true;
}

} // namespace engine
