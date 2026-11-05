/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** PlayerNotices
*/

#ifndef CLIENT_PLAYERNOTICES_HPP
#define CLIENT_PLAYERNOTICES_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace client {

/** @brief How long a notice stays on screen, in seconds. */
inline constexpr float PlayerNoticeTime = 4.0F;

/** @brief How many notices are shown at once: the oldest goes first. */
inline constexpr std::size_t MaxPlayerNotices = 4;

/**
 * @struct PlayerNotice
 * @brief A short message telling that a player joined or left the room.
 * @param slot The player's slot.
 * @param joined Whether the player joined; otherwise they left, timed out or
 * were kicked.
 * @param timeLeft How long it is still shown, in seconds.
 **/
struct PlayerNotice {
    std::uint8_t slot = 0;
    bool joined = false;
    float timeLeft = PlayerNoticeTime;
};

/**
 * @brief Adds a notice after the others, dropping the oldest one if there
 * are already MaxPlayerNotices.
 * @param notices The notices, oldest first.
 * @param slot The player's slot.
 * @param joined Whether the player joined.
 **/
inline void addPlayerNotice(std::vector<PlayerNotice> &notices,
                            std::uint8_t slot, bool joined)
{
    if (notices.size() == MaxPlayerNotices) {
        notices.erase(notices.begin());
    }
    notices.push_back({.slot = slot, .joined = joined});
}

/**
 * @brief Counts down the notices and removes those that have expired.
 * @param notices The notices, oldest first.
 * @param dt The time elapsed since the last frame, in seconds.
 **/
inline void updatePlayerNotices(std::vector<PlayerNotice> &notices, float dt)
{
    for (PlayerNotice &notice : notices) {
        notice.timeLeft -= dt;
    }
    std::erase_if(notices, [](const PlayerNotice &notice) {
        return notice.timeLeft <= 0.0F;
    });
}

} // namespace client

#endif // CLIENT_PLAYERNOTICES_HPP
