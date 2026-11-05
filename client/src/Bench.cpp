/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Bench
*/

#include "Bench.hpp"
#include "common/components/InputState.hpp"
#include "common/components/Transform.hpp"
#include "engine/Logger.hpp"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <utility>

namespace client {

namespace {

/** @brief How long the input sequence lasts before it repeats, in seconds. */
constexpr double CycleSeconds = 2.0;
/** @brief When the Up key is pressed in the sequence, in seconds. */
constexpr double UpStart = 0.7;
/** @brief When the Down key is pressed in the sequence, in seconds. */
constexpr double DownStart = 1.7;
/** @brief How long a key stays pressed, in seconds. */
constexpr double PressSeconds = 0.3;
/** @brief The time between two measures of the traffic, in seconds. */
constexpr double TrafficInterval = 1.0;
/** @brief The ship counts as moved after this distance, in playfield units. */
constexpr float MovedThreshold = 1.0F;
/** @brief Smaller prediction corrections are not recorded, in playfield units.
 */
constexpr float MinCorrection = 0.01F;
/** @brief The significant digits of the numbers in the CSV file. */
constexpr int CsvPrecision = 9;

} // namespace

Bench::Bench(BenchConfig config) : m_config(std::move(config)) {}

double Bench::elapsed(TimePoint now) const
{
    return engine::Duration{now - m_start.value_or(now)}.count();
}

void Bench::record(engine::Registry &registry, const SyncState &state,
                   const NetworkClient &network, engine::Duration dt,
                   TimePoint now)
{
    if (m_finished) {
        return;
    }
    const auto ship = state.entities.find(state.localShip);
    if (state.localShip == 0 || ship == state.entities.end()) {
        return;
    }
    if (!m_start) {
        m_start = now;
        m_receivedAtStart = network.bytesReceived();
        m_sentAtStart = network.bytesSent();
        BB_INFO("Benchmark started for " << m_config.duration.count() << " s");
    }
    const double time = elapsed(now);
    m_shipY = registry.get<rtype::Transform>(ship->second).position.y;
    m_rows.push_back({.kind = "frame", .time = time, .a = dt.count(), .b = 0});
    measureDelay(m_shipY, now);
    if (state.prediction.lastCorrection > MinCorrection) {
        m_rows.push_back({
            .kind = "correction",
            .time = time,
            .a = state.prediction.lastCorrection,
            .b = 0,
        });
    }
    measureMoves(registry, state, dt, now);
    if (time >= m_nextTraffic) {
        m_rows.push_back({
            .kind = "net",
            .time = time,
            .a = static_cast<double>(network.bytesReceived() -
                                     m_receivedAtStart),
            .b = static_cast<double>(network.bytesSent() - m_sentAtStart),
        });
        m_nextTraffic += TrafficInterval;
    }
    if (time >= m_config.duration.count()) {
        write();
        m_finished = true;
        BB_INFO("Benchmark written to " << m_config.output);
    }
}

std::uint8_t Bench::input(TimePoint now)
{
    if (!m_start || m_finished) {
        return 0;
    }
    const double phase = std::fmod(elapsed(now), CycleSeconds);
    std::uint8_t actions = 0;
    if (phase >= UpStart && phase < UpStart + PressSeconds) {
        actions = rtype::Action::Up;
    } else if (phase >= DownStart && phase < DownStart + PressSeconds) {
        actions = rtype::Action::Down;
    }
    if (actions != 0 && actions != m_lastActions) {
        if (m_press) {
            m_rows.push_back({
                .kind = "delay",
                .time = elapsed(m_press->time),
                .a = -1.0,
                .b = m_press->direction,
            });
        }
        m_press = Press{
            .time = now,
            .startY = m_shipY,
            .direction = actions == rtype::Action::Up ? -1.0F : 1.0F,
        };
    }
    m_lastActions = actions;
    return actions;
}

void Bench::measureDelay(float shipY, TimePoint now)
{
    if (!m_press ||
        (shipY - m_press->startY) * m_press->direction < MovedThreshold) {
        return;
    }
    m_rows.push_back({
        .kind = "delay",
        .time = elapsed(m_press->time),
        .a = engine::Duration{now - m_press->time}.count(),
        .b = m_press->direction,
    });
    m_press.reset();
}

void Bench::measureMoves(engine::Registry &registry, const SyncState &state,
                         engine::Duration dt, TimePoint now)
{
    std::unordered_map<std::uint32_t, engine::Vec2f> positions;
    for (const auto &[networkId, entity] : state.entities) {
        if (networkId == state.localShip) {
            continue;
        }
        const engine::Vec2f position =
            registry.get<rtype::Transform>(entity).position;
        const auto last = m_lastPositions.find(networkId);
        if (last != m_lastPositions.end()) {
            const engine::Vec2f delta = position - last->second;
            m_rows.push_back({
                .kind = "move",
                .time = elapsed(now),
                .a = std::hypot(delta.x, delta.y),
                .b = dt.count(),
            });
        }
        positions.emplace(networkId, position);
    }
    m_lastPositions = std::move(positions);
}

void Bench::write() const
{
    std::ofstream file(m_config.output);
    if (!file) {
        throw std::runtime_error("cannot write " + m_config.output);
    }
    file.precision(CsvPrecision);
    file << "kind,time,a,b\n";
    for (const Row &row : m_rows) {
        file << row.kind << ',' << row.time << ',' << row.a << ',' << row.b
             << '\n';
    }
    if (!file) {
        throw std::runtime_error("cannot write " + m_config.output);
    }
}

} // namespace client
