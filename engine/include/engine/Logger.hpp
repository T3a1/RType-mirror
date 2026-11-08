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

namespace engine {

/**
 * @enum LogLevel
 * @brief How important a log line is, from the least to the most important.
 * @param Trace Detail of the program's execution.
 * @param Debug Information useful when debugging.
 * @param Info A normal event worth keeping.
 * @param Warn Something unexpected that the program handles.
 * @param Error An operation failed.
 * @param Fatal The program cannot go on; written at once, without the queue.
 **/
enum class LogLevel : std::uint8_t { Trace, Debug, Info, Warn, Error, Fatal };

/**
 * @brief Returns the name of a level, as written in the log lines.
 * @param level The level.
 * @return Its name in capitals ("INFO"), "UNKNOWN" for an invalid value.
 **/
[[nodiscard]] std::string_view toString(LogLevel level);

/**
 * @class Logger
 * @brief Asynchronous logger, adapted from Blackbox by Thomas-Peze28
 * (https://github.com/Thomas-Peze28/Blackbox).
 *
 * The calling thread only queues the line: a writer thread formats it and
 * writes it to the console and, optionally, a file. So logging never waits
 * for the terminal or the disk, and the game loop never blocks on it. Before
 * start(), lines are written directly by the calling thread. Use it through
 * the BB_INFO family of macros.
 **/
class Logger {
  public:
    /** @brief How many lines wait in the queue by default before new ones are
     * dropped. */
    static constexpr std::size_t defaultCapacity = 4096;

    /**
     * @brief Creates a logger that is not started.
     * @param console Where the lines are written.
     * @param capacity How many lines may wait in the queue.
     **/
    explicit Logger(std::ostream &console = std::clog,
                    std::size_t capacity = defaultCapacity);
    /**
     * @brief Stops the logger, see stop().
     **/
    ~Logger();

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;

    /**
     * @brief Returns the logger shared by the whole program, the one the
     * macros use.
     * @return The logger, writing to std::clog.
     **/
    static Logger &instance();

    /**
     * @brief Starts the writer thread. Does nothing if it was already started.
     * @param minLevel Lines below this level are ignored.
     * @param file A file the lines are also appended to, created if needed;
     * none if empty.
     * @return `true` if the logger was started and the file, if any, could be
     * opened. Without the file, it still logs to the console.
     **/
    bool start(LogLevel minLevel = LogLevel::Info,
               const std::filesystem::path &file = {});

    /**
     * @brief Writes what is left in the queue, reports the dropped lines if
     * any, closes the file and stops the writer thread.
     **/
    void stop();

    /**
     * @brief Tells whether lines of a level are kept.
     * @param level The level.
     * @return `true` if it is at least the minimum level.
     **/
    [[nodiscard]] bool enabled(LogLevel level) const;

    /**
     * @brief Queues a line. If the queue is full, the line is dropped and
     * counted. A Fatal line is written at once.
     * @param level The level of the line.
     * @param file The source file it comes from.
     * @param line The line of that source file.
     * @param message The text.
     **/
    void log(LogLevel level, const char *file, int line, std::string message);

    /**
     * @brief Waits until every line queued so far is written.
     **/
    void flush();

    /**
     * @brief Sets what surrounds the console output, so that the lines of an
     * interactive console stay readable. Not written to the log file.
     * @param before Written before each line, to clear the prompt.
     * @param after Written after each batch of lines, to show the prompt again.
     **/
    void setConsoleFrame(std::string before, std::string after);

    /**
     * @brief Writes text as it is to the console, at once and without mixing
     * it with a log line, for the answers of an interactive console.
     * @param text The text.
     **/
    void print(std::string_view text);

    /**
     * @brief Builds a line only if its level is enabled, so that a disabled
     * line costs nothing, then queues it like log().
     * @param level The level of the line.
     * @param file The source file it comes from.
     * @param line The line of that source file.
     * @param format Called with a std::ostream to write the text to.
     **/
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
    std::atomic<std::uint64_t> m_queued = 0;
    std::atomic<std::uint64_t> m_written = 0;
    std::string m_frameBefore;
    std::string m_frameAfter;
    std::mutex m_writeMutex;
    std::ofstream m_file;
    std::thread m_writer;
};

} // namespace engine

// NOLINTBEGIN(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)
/**
 * @brief Logs a message at a level, with its source file and line. The message
 * is a stream expression: `BB_LOG(level, "Client " << id)`. It is not evaluated
 * if the level is disabled.
 * @param level The engine::LogLevel.
 * @param message What to write to the stream.
 **/
#define BB_LOG(level, message)                                                 \
    ::engine::Logger::instance().logWith(                                      \
        level, __FILE__, __LINE__,                                             \
        [&](std::ostream &bbStream) { bbStream << message; })

/** @brief Logs a message at the Trace level, see BB_LOG. */
#define BB_TRACE(message) BB_LOG(::engine::LogLevel::Trace, message)
/** @brief Logs a message at the Debug level, see BB_LOG. */
#define BB_DEBUG(message) BB_LOG(::engine::LogLevel::Debug, message)
/** @brief Logs a message at the Info level, see BB_LOG. */
#define BB_INFO(message) BB_LOG(::engine::LogLevel::Info, message)
/** @brief Logs a message at the Warn level, see BB_LOG. */
#define BB_WARN(message) BB_LOG(::engine::LogLevel::Warn, message)
/** @brief Logs a message at the Error level, see BB_LOG. */
#define BB_ERROR(message) BB_LOG(::engine::LogLevel::Error, message)
/** @brief Logs a message at the Fatal level, see BB_LOG. */
#define BB_FATAL(message) BB_LOG(::engine::LogLevel::Fatal, message)
// NOLINTEND(cppcoreguidelines-macro-usage, bugprone-macro-parentheses)

#endif // ENGINE_LOGGER_HPP
