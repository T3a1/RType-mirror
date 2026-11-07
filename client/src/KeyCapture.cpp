/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** KeyCapture
*/

#include "KeyCapture.hpp"

namespace client {

namespace {

/**
 * @brief Returns the key of the keyboard held, if any.
 * @param graphics The graphics backend.
 * @return The first key held, leaving out Escape and the gamepad, whose
 * buttons are not keyboard keys.
 **/
std::optional<gfx::Key> heldKey(const gfx::IGraphics &graphics)
{
    for (std::size_t i = 0; i < gfx::KeyCount; ++i) {
        const auto key = static_cast<gfx::Key>(i);
        if (key == gfx::Key::Escape || gfx::isPadKey(key)) {
            continue;
        }
        if (graphics.isKeyDown(key)) {
            return key;
        }
    }
    return std::nullopt;
}

} // namespace

void KeyCapture::start(KeySlot slot)
{
    m_slot = slot;
    m_released = false;
}

void KeyCapture::cancel() { m_slot.reset(); }

std::optional<KeySlot> KeyCapture::slot() const { return m_slot; }

std::optional<gfx::Key> KeyCapture::update(const gfx::IGraphics &graphics)
{
    if (!m_slot) {
        return std::nullopt;
    }
    const std::optional<gfx::Key> held = heldKey(graphics);
    if (!m_released) {
        m_released = !held;
        return std::nullopt;
    }
    return held;
}

} // namespace client
