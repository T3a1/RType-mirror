/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Chat
*/

#include "Chat.hpp"
#include "common/protocol/Messages.hpp"
#include "graphics/Types.hpp"

#include <utility>

namespace client {

namespace {

/** @brief The first accented letter of Latin-1: À. */
constexpr char32_t LatinFirst = 0xC0;
/** @brief The last letter of Latin-1: ÿ. */
constexpr char32_t LatinLast = 0xFF;
/** @brief The letter Œ, outside of Latin-1. */
constexpr char32_t UpperOe = 0x152;
/** @brief The letter œ, outside of Latin-1. */
constexpr char32_t LowerOe = 0x153;
/** @brief The ASCII equivalent of each Latin-1 character from LatinFirst to
 * LatinLast. */
constexpr std::string_view LatinToAscii =
    "AAAAAAACEEEEIIIIDNOOOOO*OUUUUYTsaaaaaaaceeeeiiiidnooooo/ouuuuyty";

} // namespace

void addChatLine(ChatState &chat, std::uint8_t slot, std::string text)
{
    if (chat.lines.size() == MaxChatLines) {
        chat.lines.erase(chat.lines.begin());
    }
    chat.lines.push_back({.slot = slot, .text = std::move(text)});
}

void updateChatLines(ChatState &chat, float dt)
{
    if (chat.open) {
        return;
    }
    for (ChatLine &line : chat.lines) {
        line.timeLeft -= dt;
    }
    std::erase_if(chat.lines,
                  [](const ChatLine &line) { return line.timeLeft <= 0.0F; });
}

std::optional<char> toChatCharacter(char32_t character)
{
    if (character >= LatinFirst && character <= LatinLast) {
        return LatinToAscii.at(character - LatinFirst);
    }
    if (character == UpperOe || character == LowerOe) {
        return character == UpperOe ? 'O' : 'o';
    }
    if (character > U'~' || character < U' ') {
        return std::nullopt;
    }
    return static_cast<char>(character);
}

void typeInDraft(std::string &draft, std::u32string_view typed)
{
    for (const char32_t character : typed) {
        if (character == gfx::TextBackspace) {
            if (!draft.empty()) {
                draft.pop_back();
            }
        } else if (const auto converted = toChatCharacter(character);
                   converted && draft.size() < rtype::protocol::MaxChatLength) {
            draft.push_back(*converted);
        }
    }
}

std::optional<std::string> closeChat(ChatState &chat)
{
    const std::size_t first = chat.draft.find_first_not_of(' ');
    std::optional<std::string> line;
    if (first != std::string::npos) {
        const std::size_t last = chat.draft.find_last_not_of(' ');
        line = chat.draft.substr(first, last - first + 1);
    }
    chat.open = false;
    chat.draft.clear();
    return line;
}

} // namespace client
