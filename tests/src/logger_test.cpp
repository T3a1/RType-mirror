/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** logger_test
*/

#include "engine/Logger.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

namespace {

std::size_t countOf(const std::string &text, const std::string &word)
{
    std::size_t count = 0;
    for (std::size_t pos = text.find(word); pos != std::string::npos;
         pos = text.find(word, pos + word.size())) {
        ++count;
    }
    return count;
}

std::string readFile(const std::filesystem::path &path)
{
    std::ifstream file(path);
    return {std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
}

std::filesystem::path tempPath(const std::string &name)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / ("rtype_" + name);
    std::filesystem::remove_all(path);
    return path;
}

} // namespace

TEST(Logger, FormatsLevelFileLineAndMessage)
{
    std::ostringstream out;
    engine::Logger logger(out);
    logger.start(engine::LogLevel::Trace);
    logger.log(engine::LogLevel::Warn, "/path/to/Game.cpp", 42, "Low health");
    logger.stop();

    EXPECT_NE(out.str().find("[WARN] [Game.cpp:42] Low health\n"),
              std::string::npos);
}

TEST(Logger, SkipsLevelsBelowTheMinimum)
{
    std::ostringstream out;
    engine::Logger logger(out);
    logger.start(engine::LogLevel::Warn);
    logger.log(engine::LogLevel::Info, "a.cpp", 1, "hidden");
    logger.log(engine::LogLevel::Error, "a.cpp", 2, "shown");
    logger.stop();

    EXPECT_FALSE(logger.enabled(engine::LogLevel::Info));
    EXPECT_EQ(out.str().find("hidden"), std::string::npos);
    EXPECT_NE(out.str().find("shown"), std::string::npos);
}

TEST(Logger, WritesEveryQueuedLineOnStop)
{
    std::ostringstream out;
    engine::Logger logger(out);
    logger.start(engine::LogLevel::Info);
    for (int i = 0; i < 1000; ++i) {
        logger.log(engine::LogLevel::Info, "a.cpp", i, "line");
    }
    logger.stop();

    EXPECT_EQ(countOf(out.str(), "line\n"), 1000U);
}

TEST(Logger, LinesBeforeStartAndAfterStopAreNotLost)
{
    std::ostringstream out;
    engine::Logger logger(out);
    logger.log(engine::LogLevel::Info, "a.cpp", 1, "before start");
    logger.start(engine::LogLevel::Info);
    logger.stop();
    logger.log(engine::LogLevel::Info, "a.cpp", 2, "after stop");

    EXPECT_NE(out.str().find("before start"), std::string::npos);
    EXPECT_NE(out.str().find("after stop"), std::string::npos);
}

TEST(Logger, StartsOnlyOnce)
{
    std::ostringstream out;
    engine::Logger logger(out);

    EXPECT_TRUE(logger.start(engine::LogLevel::Info));
    EXPECT_FALSE(logger.start(engine::LogLevel::Info));
    logger.stop();
}

TEST(Logger, AlsoWritesToTheFile)
{
    const std::filesystem::path dir = tempPath("logger_file");
    std::ostringstream out;
    engine::Logger logger(out);

    EXPECT_TRUE(logger.start(engine::LogLevel::Info, dir / "logs/server.log"));
    logger.log(engine::LogLevel::Info, "a.cpp", 1, "to the file");
    logger.stop();

    EXPECT_NE(readFile(dir / "logs/server.log").find("to the file"),
              std::string::npos);
    std::filesystem::remove_all(dir);
}

TEST(Logger, UnwritableFileFallsBackToTheConsole)
{
    // A regular file used as a directory: the log file cannot be created.
    const std::filesystem::path notADirectory = tempPath("logger_not_a_dir");
    std::ofstream(notADirectory) << "x";
    std::ostringstream out;
    engine::Logger logger(out);

    EXPECT_FALSE(
        logger.start(engine::LogLevel::Info, notADirectory / "server.log"));
    logger.log(engine::LogLevel::Info, "a.cpp", 1, "still logged");
    logger.stop();

    EXPECT_NE(out.str().find("Cannot open the log file"), std::string::npos);
    EXPECT_NE(out.str().find("still logged"), std::string::npos);
    std::filesystem::remove_all(notADirectory);
}

TEST(Logger, ReportsDroppedLinesWhenTheQueueIsFull)
{
    std::ostringstream out;
    // Capacity 0: every queued line is dropped and counted.
    engine::Logger logger(out, 0);
    logger.start(engine::LogLevel::Info);
    logger.log(engine::LogLevel::Info, "a.cpp", 1, "lost");
    logger.log(engine::LogLevel::Info, "a.cpp", 2, "lost");
    logger.stop();

    EXPECT_EQ(out.str().find("lost"), std::string::npos);
    EXPECT_NE(out.str().find("2 log lines dropped"), std::string::npos);
}

TEST(Logger, FatalIsWrittenEvenWithAFullQueue)
{
    std::ostringstream out;
    engine::Logger logger(out, 0);
    logger.start(engine::LogLevel::Info);
    logger.log(engine::LogLevel::Fatal, "a.cpp", 1, "crash");
    logger.stop();

    EXPECT_NE(out.str().find("[FATAL] [a.cpp:1] crash"), std::string::npos);
}
