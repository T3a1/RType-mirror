/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** KeyCapture
*/

#ifndef CLIENT_KEYCAPTURE_HPP
#define CLIENT_KEYCAPTURE_HPP

#include "Controls.hpp"
#include "graphics/IGraphics.hpp"

#include <cstddef>
#include <optional>

namespace client {

/**
 * @struct KeySlot
 * @brief One place among the keys of a control.
 * @param control The control.
 * @param index The place among its keys.
 **/
struct KeySlot {
    Control control = Control::Up;
    std::size_t index = 0;
};

/**
 * @class KeyCapture
 * @brief Waits for the player to press the key to bind to a control.
 *
 * The key that started the capture is still held at that moment, so it first
 * waits for every key to be released. Escape is never captured: it cancels.
 * Only keyboard keys are captured: a gamepad button is ignored.
 **/
class KeyCapture {
  public:
    /**
     * @brief Starts waiting for a key.
     * @param slot Where the key goes.
     **/
    void start(KeySlot slot);
    /**
     * @brief Stops waiting.
     **/
    void cancel();
    /**
     * @brief Returns where the key goes.
     * @return The place, or nothing if it is not waiting.
     **/
    [[nodiscard]] std::optional<KeySlot> slot() const;
    /**
     * @brief Reads the keyboard: call it once per frame while it is waiting.
     * @param graphics The graphics backend.
     * @return The key pressed, if any.
     **/
    [[nodiscard]] std::optional<gfx::Key>
    update(const gfx::IGraphics &graphics);

  private:
    std::optional<KeySlot> m_slot;
    bool m_released = false;
};

} // namespace client

#endif // CLIENT_KEYCAPTURE_HPP
