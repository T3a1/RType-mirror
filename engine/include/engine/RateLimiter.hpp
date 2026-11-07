/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** RateLimiter
*/

#ifndef ENGINE_RATELIMITER_HPP
#define ENGINE_RATELIMITER_HPP

#include <algorithm>
#include <chrono>
#include <optional>

namespace engine {

/**
 * @class RateLimiter
 * @brief Limits how often something may happen: a short burst is accepted,
 * then a steady rate (a token bucket). It keeps a client from flooding a
 * server, for example. It reads no clock, so that tests can choose the time.
 **/
class RateLimiter {
  public:
    using TimePoint = std::chrono::steady_clock::time_point;

    /**
     * @brief Creates a limiter that accepts a full burst at once.
     * @param burst How many events may happen in a row.
     * @param perSecond How many events per second are accepted after that.
     **/
    RateLimiter(float burst, float perSecond)
        : m_burst(burst), m_perSecond(perSecond), m_tokens(burst)
    {
    }

    /**
     * @brief Asks for one event.
     * @param now The current time.
     * @return `true` if it is accepted.
     **/
    [[nodiscard]] bool allow(TimePoint now)
    {
        if (m_last) {
            const std::chrono::duration<float> elapsed = now - *m_last;
            m_tokens =
                std::min(m_burst, m_tokens + (elapsed.count() * m_perSecond));
        }
        m_last = now;
        if (m_tokens < 1.0F) {
            return false;
        }
        m_tokens -= 1.0F;
        return true;
    }

  private:
    float m_burst;
    float m_perSecond;
    float m_tokens;
    std::optional<TimePoint> m_last;
};

} // namespace engine

#endif // ENGINE_RATELIMITER_HPP
