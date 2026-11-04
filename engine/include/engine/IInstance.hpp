/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** IInstance
*/

#ifndef ENGINE_IINSTANCE_HPP
#define ENGINE_IINSTANCE_HPP

#include "engine/Clock.hpp"

#include <cstdint>

namespace engine {

/**
 * @class IInstance
 * @brief One independent run of a game, hosted by an InstanceManager on its
 * own thread: a game implements it and owns its registry and its systems.
 **/
class IInstance {
  public:
    IInstance() = default;
    virtual ~IInstance() = default;

    IInstance(const IInstance &) = delete;
    IInstance &operator=(const IInstance &) = delete;
    IInstance(IInstance &&) = delete;
    IInstance &operator=(IInstance &&) = delete;

    /**
     * @brief Advances the game by one fixed time step. An exception ends
     * this instance only.
     * @param dt The time step.
     **/
    virtual void tick(Duration dt) = 0;

    /**
     * @brief Called once on the instance's thread, after its last tick.
     **/
    virtual void onStop() {}

    /**
     * @brief Returns whether new members may join. Called from the thread
     * of the InstanceManager while tick() runs: it must be thread-safe.
     * @return True by default.
     **/
    [[nodiscard]] virtual bool joinable() const { return true; }

    /**
     * @brief Returns where the game stands, in a form only the game
     * interprets, for a list of the running instances. Thread-safe, like
     * joinable().
     * @return 0 by default.
     **/
    [[nodiscard]] virtual std::uint8_t status() const { return 0; }
};

} // namespace engine

#endif // ENGINE_IINSTANCE_HPP
