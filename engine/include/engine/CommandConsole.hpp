/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** CommandConsole
*/

#ifndef ENGINE_COMMANDCONSOLE_HPP
#define ENGINE_COMMANDCONSOLE_HPP

#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

/**
 * @class CommandConsole
 * @brief Interprets lines of text as named commands, for an administration
 * console. It does no input or output and knows nothing about the game: the
 * program registers its commands, and decides where lines come from and where
 * the answers go. It has a built-in `help` command.
 **/
class CommandConsole {
  public:
    /** @brief The arguments of a command, without its name. */
    using Args = std::vector<std::string>;
    /** @brief Runs a command and returns its answer; may throw UsageError. */
    using Handler = std::function<std::string(const Args &)>;

    /**
     * @class UsageError
     * @brief Thrown by a handler when its arguments are wrong: the console
     * answers with the usage of the command.
     **/
    class UsageError : public std::runtime_error {
      public:
        /**
         * @brief Creates the error.
         **/
        UsageError() : std::runtime_error("wrong arguments") {}
    };

    /**
     * @brief Registers a command, replacing any command of the same name.
     * @param name What the user types.
     * @param usage Its arguments, as shown in the help ("<id>").
     * @param help What it does, in a few words.
     * @param handler Runs it and returns its answer.
     **/
    void add(const std::string &name, const std::string &usage,
             const std::string &help, Handler handler);

    /**
     * @brief Runs one line: the first word is the command, the others its
     * arguments. An unknown command or a failing handler gives an error
     * message, never an exception.
     * @param line The line.
     * @return The answer, empty for a blank line.
     **/
    [[nodiscard]] std::string execute(std::string_view line) const;

  private:
    struct Command {
        std::string usage;
        std::string help;
        Handler handler;
    };

    [[nodiscard]] std::string helpText() const;

    std::map<std::string, Command> m_commands;
};

} // namespace engine

#endif // ENGINE_COMMANDCONSOLE_HPP
