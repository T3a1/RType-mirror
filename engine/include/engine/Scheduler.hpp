/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Scheduler
*/

#ifndef ENGINE_SCHEDULER_HPP
#define ENGINE_SCHEDULER_HPP

#include "engine/Clock.hpp"
#include "engine/Registry.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace engine {

/**
 * @enum Stage
 * @brief A step of the tick. Every tick runs the stages in STAGE_ORDER.
 * @param Input Reads what comes from outside and decides.
 * @param Simulation Moves the world forward.
 * @param Cleanup Applies the deferred changes; the registry is flushed after.
 * @param Output Sends the finished world.
 **/
enum class Stage : std::uint8_t {
    Input,
    Simulation,
    Cleanup,
    Output,
};

/** @brief The order in which the stages run every tick. */
inline constexpr std::array<Stage, 4> STAGE_ORDER = {
    Stage::Input,
    Stage::Simulation,
    Stage::Cleanup,
    Stage::Output,
};

/** @brief A system as the scheduler sees it. Extra dependencies are captured.
 */
using System = std::function<void(Registry &, Duration)>;

/** @brief Tells whether a system runs on this tick. */
using Condition = std::function<bool()>;

/**
 * @struct SystemOptions
 * @brief When a system runs. By default, on every tick.
 * @param every Runs the system once every this many ticks (1 = every tick,
 * 0 is treated as 1).
 * @param when Runs the system only while this returns true, if set. It is
 * checked on the ticks selected by every.
 **/
struct SystemOptions {
    std::uint32_t every = 1;
    Condition when = nullptr;
};

/**
 * @class Scheduler
 * @brief Owns the systems of a game and runs them in order every tick.
 *
 * Systems run stage by stage, in STAGE_ORDER, and in the order they were
 * added within a stage. The registry is flushed after the Cleanup stage.
 **/
class Scheduler {
  public:
    /**
     * @brief Adds a system to a stage.
     * @param stage The stage the system runs in.
     * @param name The name of the system, for logs and debugging.
     * @param system The system to run.
     * @param options When the system runs.
     **/
    void add(Stage stage, std::string name, System system,
             SystemOptions options = {});

    /**
     * @brief Runs one tick: every stage in order, then flushes the
     * registry after the Cleanup stage.
     * @param registry The registry the systems work on.
     * @param dt The time elapsed since the last tick.
     **/
    void run(Registry &registry, Duration dt);

  private:
    struct Entry {
        Stage stage;
        std::string name;
        System system;
        std::uint32_t every;
        Condition when;
    };

    std::vector<Entry> m_systems;
    std::uint64_t m_tick = 0;
};

} // namespace engine

#endif // ENGINE_SCHEDULER_HPP
