/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Bench
*/

#ifndef CLIENT_BENCH_HPP
#define CLIENT_BENCH_HPP

#include "engine/Clock.hpp"
#include "engine/Registry.hpp"
#include "engine/Vec2.hpp"
#include "network/NetworkClient.hpp"
#include "systems/NetworkSyncSystem.hpp"

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace client {

/**
 * @struct BenchConfig
 * @brief How the client runs the network benchmark.
 * @param duration How long to measure, from the first frame with the local
 * ship.
 * @param output The CSV file the measures are written to.
 **/
struct BenchConfig {
    engine::Duration duration{};
    std::string output;
};

/**
 * @class Bench
 * @brief The network benchmark: plays a fixed input sequence instead of the
 * keyboard, measures what the player sees, and writes it to a CSV file.
 *
 * The sequence repeats every 2 s: idle, Up for 0.3 s, idle, Down for 0.3 s.
 * Each CSV row is `kind,time,a,b`, time in seconds since the start:
 * - `frame,t,dt,0`: one displayed frame and its duration;
 * - `move,t,distance,dt`: how far one synced entity (all but the local ship)
 *   moved on screen since the last frame, in playfield units;
 * - `delay,t,seconds,direction`: time between a key press and the first
 *   frame showing the local ship moving that way (-1 if it never did before
 *   the next press); direction is -1 for Up, 1 for Down;
 * - `net,t,received,sent`: the bytes received and sent since the start,
 *   once per second.
 **/
class Bench {
  public:
    using TimePoint = std::chrono::steady_clock::time_point;

    /**
     * @brief Prepares a benchmark; it starts once the local ship exists.
     * @param config The duration and the output file.
     **/
    explicit Bench(BenchConfig config);

    /**
     * @brief Measures the frame about to be drawn. Starts the benchmark on
     * the first frame with the local ship, and writes the CSV file once the
     * duration is over. Call it after the sync and smoothing systems.
     * @param registry The client's registry.
     * @param state The sync state.
     * @param network The connection, for the traffic counters.
     * @param dt The duration of the frame.
     * @param now The current time.
     * @throws std::runtime_error If the CSV file cannot be written.
     **/
    void record(engine::Registry &registry, const SyncState &state,
                const NetworkClient &network, engine::Duration dt,
                TimePoint now);

    /**
     * @brief Returns the actions of the input sequence at this frame, and
     * notes the start of each key press.
     * @param now The current time.
     * @return The held actions, as Action bits; none before the start.
     **/
    [[nodiscard]] std::uint8_t input(TimePoint now);

    /**
     * @brief Returns whether the measures are over and written.
     * @return True once the CSV file is written.
     **/
    [[nodiscard]] bool finished() const { return m_finished; }

  private:
    struct Row {
        const char *kind;
        double time;
        double a;
        double b;
    };

    struct Press {
        TimePoint time;
        float startY;
        float direction;
    };

    [[nodiscard]] double elapsed(TimePoint now) const;
    void measureDelay(float shipY, TimePoint now);
    void measureMoves(engine::Registry &registry, const SyncState &state,
                      engine::Duration dt, TimePoint now);
    void write() const;

    BenchConfig m_config;
    std::optional<TimePoint> m_start;
    std::optional<Press> m_press;
    float m_shipY = 0.0F;
    std::uint8_t m_lastActions = 0;
    std::unordered_map<std::uint32_t, engine::Vec2f> m_lastPositions;
    double m_nextTraffic = 0.0;
    std::uint64_t m_receivedAtStart = 0;
    std::uint64_t m_sentAtStart = 0;
    std::vector<Row> m_rows;
    bool m_finished = false;
};

} // namespace client

#endif // CLIENT_BENCH_HPP
