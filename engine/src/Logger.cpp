/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** Logger
*/

#include "engine/Logger.hpp"

#include <ctime>
#include <iomanip>
#include <optional>
#include <system_error>
#include <utility>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#endif

namespace engine {

namespace {

bool supportsColors(const std::ostream &console)
{
#if defined(__unix__) || defined(__APPLE__)
    if (&console == &std::cout) {
        return isatty(STDOUT_FILENO) == 1;
    }
    if (&console == &std::clog || &console == &std::cerr) {
        return isatty(STDERR_FILENO) == 1;
    }
#endif
    static_cast<void>(console);
    return false;
}

std::string_view colorFor(LogLevel level)
{
    switch (level) {
    case LogLevel::Trace:
        return "\033[90m";
    case LogLevel::Debug:
        return "\033[36m";
    case LogLevel::Info:
        return "\033[32m";
    case LogLevel::Warn:
        return "\033[33m";
    case LogLevel::Error:
        return "\033[31m";
    case LogLevel::Fatal:
        return "\033[35m";
    }
    return "";
}

constexpr std::string_view resetColor = "\033[0m";

std::string_view fileName(std::string_view path)
{
    const std::size_t slash = path.find_last_of("/\\");
    return slash == std::string_view::npos ? path : path.substr(slash + 1);
}

} // namespace

std::string_view toString(LogLevel level)
{
    switch (level) {
    case LogLevel::Trace:
        return "TRACE";
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warn:
        return "WARN";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Fatal:
        return "FATAL";
    }
    return "UNKNOWN";
}

Logger::Logger(std::ostream &console, std::size_t capacity)
    : m_console(&console), m_colors(supportsColors(console)), m_queue(capacity)
{
}

Logger::~Logger() { stop(); }

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

bool Logger::start(LogLevel minLevel, const std::filesystem::path &file)
{
    if (m_started.exchange(true)) {
        return false;
    }
    m_minLevel = minLevel;

    bool fileOpened = true;
    if (!file.empty()) {
        std::error_code error;
        if (file.has_parent_path()) {
            std::filesystem::create_directories(file.parent_path(), error);
        }
        {
            const std::scoped_lock lock(m_writeMutex);
            m_file.open(file, std::ios::app);
            fileOpened = m_file.is_open();
        }
        if (!fileOpened) {
            log(LogLevel::Warn, __FILE__, __LINE__,
                "Cannot open the log file " + file.string() +
                    ", logging to the console only");
        }
    }

    m_running = true;
    m_writer = std::thread(&Logger::run, this);
    return fileOpened;
}

void Logger::stop()
{
    if (!m_running.exchange(false)) {
        return;
    }
    m_queue.close();
    if (m_writer.joinable()) {
        m_writer.join();
    }
    const std::scoped_lock lock(m_writeMutex);
    reportDropped();
    m_console->flush();
    if (m_file.is_open()) {
        m_file.close();
    }
}

bool Logger::enabled(LogLevel level) const { return level >= m_minLevel; }

void Logger::log(LogLevel level, const char *file, int line,
                 std::string message)
{
    if (!enabled(level)) {
        return;
    }
    Entry entry{
        .level = level,
        .time = std::chrono::system_clock::now(),
        .file = file,
        .line = line,
        .message = std::move(message),
    };

    if (m_running && level != LogLevel::Fatal) {
        if (m_queue.push(std::move(entry))) {
            return;
        }
        if (!m_queue.closed()) {
            ++m_dropped;
            return;
        }
    }

    const std::scoped_lock lock(m_writeMutex);
    // NOLINTNEXTLINE(bugprone-use-after-move)
    write(entry);
    m_console->flush();
    if (m_file.is_open()) {
        m_file.flush();
    }
}

void Logger::run()
{
    while (std::optional<Entry> first = m_queue.waitPop()) {
        const std::vector<Entry> rest = m_queue.drain();
        const std::scoped_lock lock(m_writeMutex);
        write(*first);
        for (const Entry &entry : rest) {
            write(entry);
        }
        reportDropped();
        m_console->flush();
        if (m_file.is_open()) {
            m_file.flush();
        }
    }
}

void Logger::write(const Entry &entry)
{
    const std::time_t time = std::chrono::system_clock::to_time_t(entry.time);
    std::tm local{};
    localtime_r(&time, &local);

    std::ostringstream stream;
    stream << std::put_time(&local, "%Y-%m-%d %H:%M:%S") << " ["
           << toString(entry.level) << "] [" << fileName(entry.file) << ':'
           << entry.line << "] " << entry.message;
    const std::string text = std::move(stream).str();

    if (m_colors) {
        *m_console << colorFor(entry.level) << text << resetColor << '\n';
    } else {
        *m_console << text << '\n';
    }
    if (m_file.is_open()) {
        m_file << text << '\n';
    }
}

void Logger::reportDropped()
{
    const std::uint64_t dropped = m_dropped.exchange(0);
    if (dropped == 0) {
        return;
    }
    write(Entry{
        .level = LogLevel::Warn,
        .time = std::chrono::system_clock::now(),
        .file = __FILE__,
        .line = __LINE__,
        .message =
            std::to_string(dropped) + " log lines dropped: the queue was full",
    });
}

} // namespace engine
