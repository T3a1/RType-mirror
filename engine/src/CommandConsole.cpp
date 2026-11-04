/*
** EPITECH PROJECT, 2026
** r-type_engine
** File description:
** CommandConsole
*/

#include "engine/CommandConsole.hpp"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <sstream>
#include <utility>

namespace engine {

namespace {

/**
 * @brief Returns how a command is typed.
 * @param name Its name.
 * @param usage Its arguments, possibly none.
 * @return The name and the arguments.
 **/
std::string signature(const std::string &name, const std::string &usage)
{
    return usage.empty() ? name : name + " " + usage;
}

} // namespace

void CommandConsole::add(const std::string &name, const std::string &usage,
                         const std::string &help, Handler handler)
{
    m_commands[name] = Command{
        .usage = usage,
        .help = help,
        .handler = std::move(handler),
    };
}

std::string CommandConsole::execute(std::string_view line) const
{
    std::istringstream words{std::string{line}};
    std::string name;
    if (!(words >> name)) {
        return {};
    }
    if (name == "help") {
        return helpText();
    }
    const auto command = m_commands.find(name);
    if (command == m_commands.end()) {
        return "Unknown command: " + name + ". Type help for the list.";
    }
    Args args;
    for (std::string word; words >> word;) {
        args.push_back(word);
    }
    try {
        return command->second.handler(args);
    } catch (const UsageError &) {
        return "Usage: " + signature(name, command->second.usage);
    } catch (const std::exception &error) {
        return std::string{"Error: "} + error.what();
    }
}

std::string CommandConsole::helpText() const
{
    std::size_t width = std::string_view{"help"}.size();
    for (const auto &[name, command] : m_commands) {
        width = std::max(width, signature(name, command.usage).size());
    }
    std::ostringstream text;
    text << std::left << std::setw(static_cast<int>(width) + 2) << "help"
         << "List the commands.";
    for (const auto &[name, command] : m_commands) {
        text << '\n'
             << std::setw(static_cast<int>(width) + 2)
             << signature(name, command.usage) << command.help;
    }
    return text.str();
}

} // namespace engine
