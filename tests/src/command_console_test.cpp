/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** command_console_test
*/

#include "engine/CommandConsole.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

namespace {

using engine::CommandConsole;

CommandConsole makeConsole()
{
    CommandConsole console;
    console.add("echo", "<word>", "Repeat a word.",
                [](const CommandConsole::Args &args) {
                    if (args.size() != 1) {
                        throw CommandConsole::UsageError();
                    }
                    return args.front();
                });
    console.add("fail", "", "Always fails.",
                [](const CommandConsole::Args &) -> std::string {
                    throw std::runtime_error("broken");
                });
    return console;
}

} // namespace

TEST(CommandConsole, RunsACommandWithItsArguments)
{
    EXPECT_EQ(makeConsole().execute("  echo   hello "), "hello");
}

TEST(CommandConsole, ABlankLineAnswersNothing)
{
    EXPECT_EQ(makeConsole().execute("   "), "");
}

TEST(CommandConsole, AnUnknownCommandIsAnError)
{
    EXPECT_EQ(makeConsole().execute("nope"),
              "Unknown command: nope. Type help for the list.");
}

TEST(CommandConsole, WrongArgumentsShowTheUsage)
{
    EXPECT_EQ(makeConsole().execute("echo"), "Usage: echo <word>");
}

TEST(CommandConsole, AFailingCommandDoesNotThrow)
{
    EXPECT_EQ(makeConsole().execute("fail"), "Error: broken");
}

TEST(CommandConsole, HelpListsTheCommands)
{
    const std::string help = makeConsole().execute("help");

    EXPECT_NE(help.find("echo <word>  Repeat a word."), std::string::npos);
    EXPECT_NE(help.find("fail         Always fails."), std::string::npos);
    EXPECT_NE(help.find("help         List the commands."), std::string::npos);
}
