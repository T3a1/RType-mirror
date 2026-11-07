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
 * @brief Returns the key of a device held, if any.
 * @param graphics The graphics backend.
 * @param device The device.
 * @return The first key of the device held, leaving out Escape.
 **/
std::optional<gfx::Key> heldKey(const gfx::IGraphics &graphics, Device device)
{
    const bool gamepad = device == Device::Gamepad;
    for (std::size_t i = 0; i < gfx::KeyCount; ++i) {
        const auto key = static_cast<gfx::Key>(i);
        if (key == gfx::Key::Escape || gfx::isPadKey(key) != gamepad) {
            continue;
        }
        if (graphics.isKeyDown(key)) {
            return key;
        }
    }
    return std::nullopt;
}

} // namespace

void KeyCapture::start(KeySlot slot, Device device)
{
    m_slot = slot;
    m_device = device;
    m_released = false;
}

void KeyCapture::cancel() { m_slot.reset(); }

std::optional<KeySlot> KeyCapture::slot() const { return m_slot; }

Device KeyCapture::device() const { return m_device; }

std::optional<gfx::Key> KeyCapture::update(const gfx::IGraphics &graphics)
{
    if (!m_slot) {
        return std::nullopt;
    }
    const std::optional<gfx::Key> held = heldKey(graphics, m_device);
    if (!m_released) {
        m_released = !held;
        return std::nullopt;
    }
    return held;
}

} // namespace client
