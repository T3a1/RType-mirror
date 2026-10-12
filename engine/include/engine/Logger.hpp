/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Logger
*/

#ifndef ENGINE_LOGGER_HPP
#define ENGINE_LOGGER_HPP

#include "engine/MessageQueue.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

// Asynchronous logger, adapted from Blackbox by Thomas-Peze28
// (https://github.com/Thomas-Peze28/Blackbox).
//
// The calling thread only queues the line: a writer thread formats it and
// writes it to the console and, optionally, a file. So logging never waits
// for the terminal or the disk, and the game loop never blocks on it.

namespace engine {

enum class LogLevel : std::uint8_t { Trace, Debug, Info, Warn, Error, Fatal };

[[nodiscard]] std::string_view toString(LogLevel level);

class Logger {
  public:
    static constexpr std::size_t defaultCapacity = 4096;

    explicit Logger(std::ostream &console = std::clog,
                    std::size_t capacity = defaultCapacity);
    ~Logger();

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;

    static Logger &instance();

    bool start(LogLevel minLevel = LogLevel::Info,
               const std::filesystem::path &file = {});

    void stop();

    [[nodiscard]] bool enabled(LogLevel level) const;

    void log(LogLevel level, const char *file, int line, std::string message);

    // Runs `format` on a stream only if the level is enabled.
    template <typename Format>
    void logWith(LogLevel level, const char *file, int line, Format &&format)
    {
        if (!enabled(level)) {
            return;
        }
        std::ostringstream stream;
        std::forward<Format>(format)(stream);
        log(level, file, line, std::move(stream).str());
    }

  private:
    struct Entry {
        LogLevel level = LogLevel::Info;
        std::chrono::system_clock::time_point time;
        const char *file = "";
        int line = 0;
        std::string message;
    };

    void run();
    void write(const Entry &entry);
    void reportDropped();

    std::ostream *m_console;
    bool m_colors;
    MessageQueue<Entry> m_queue;
    std::atomic<LogLevel> m_minLevel = LogLevel::Info;
    std::atomic<bool> m_started = false;
    std::atomic<bool> m_running = false;
    std::atomic<std::uint64_t> m_dropped = 0;
    std::mutex m_writeMutex;
    std::ofstream m_file;
    std::thread m_writer;
};

} // namespace engine

// NOLINTBEGIN(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)
#define BB_LOG(level, message)                                                 \
    ::engine::Logger::instance().logWith(                                      \
        level, __FILE__, __LINE__,                                             \
        [&](std::ostream &bbStream) { bbStream << message; })

#define BB_TRACE(message) BB_LOG(::engine::LogLevel::Trace, message)
#define BB_DEBUG(message) BB_LOG(::engine::LogLevel::Debug, message)
#define BB_INFO(message) BB_LOG(::engine::LogLevel::Info, message)
#define BB_WARN(message) BB_LOG(::engine::LogLevel::Warn, message)
#define BB_ERROR(message) BB_LOG(::engine::LogLevel::Error, message)
#define BB_FATAL(message) BB_LOG(::engine::LogLevel::Fatal, message)
// NOLINTEND(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)

#endif // ENGINE_LOGGER_HPP
