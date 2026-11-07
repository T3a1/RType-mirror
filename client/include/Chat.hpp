/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Chat
*/

#ifndef CLIENT_CHAT_HPP
#define CLIENT_CHAT_HPP

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace client {

/** @brief How long a line of chat stays on screen, in seconds. */
inline constexpr float ChatLineTime = 10.0F;

/** @brief How many lines of chat are shown at once: the oldest goes first. */
inline constexpr std::size_t MaxChatLines = 6;

/**
 * @struct ChatLine
 * @brief A line received from the server.
 * @param slot The author's slot.
 * @param text What they wrote.
 * @param timeLeft How long it is still shown, in seconds.
 **/
struct ChatLine {
    std::uint8_t slot = 0;
    std::string text;
    float timeLeft = ChatLineTime;
};

/**
 * @struct ChatState
 * @brief The chat of the current room.
 * @param open Whether the player is typing: the keyboard then goes to the chat
 * only.
 * @param draft What the player has typed so far.
 * @param lines The lines received, oldest first.
 **/
struct ChatState {
    bool open = false;
    std::string draft;
    std::vector<ChatLine> lines;
};

/**
 * @brief Adds a received line after the others, dropping the oldest one if
 * there are already MaxChatLines.
 * @param chat The chat.
 * @param slot The author's slot.
 * @param text What they wrote.
 **/
void addChatLine(ChatState &chat, std::uint8_t slot, std::string text);

/**
 * @brief Counts down the lines and removes those that have expired. Time stops
 * while the player is typing, so that the lines they answer stay on screen.
 * @param chat The chat.
 * @param dt The time elapsed since the last frame, in seconds.
 **/
void updateChatLines(ChatState &chat, float dt);

/**
 * @brief Returns the character of the protocol for a typed character: accented
 * Latin letters lose their accent, since the protocol only carries printable
 * ASCII.
 * @param character A character typed by the player.
 * @return The chat character, or nothing if it has none.
 **/
[[nodiscard]] std::optional<char> toChatCharacter(char32_t character);

/**
 * @brief Applies what the player typed to the draft: gfx::TextBackspace removes
 * the last character, and characters beyond MaxChatLength are ignored.
 * @param draft The draft, updated.
 * @param typed The characters typed, in order.
 **/
void typeInDraft(std::string &draft, std::u32string_view typed);

/**
 * @brief Closes the chat and empties the draft.
 * @param chat The chat.
 * @return The line to send: the draft without its outer spaces, or nothing if
 * it holds nothing else.
 **/
[[nodiscard]] std::optional<std::string> closeChat(ChatState &chat);

} // namespace client

#endif // CLIENT_CHAT_HPP
