/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** NcursesGraphics
*/

#ifndef CLIENT_GRAPHICS_NCURSESGRAPHICS_HPP
#define CLIENT_GRAPHICS_NCURSESGRAPHICS_HPP

#include "graphics/IGraphics.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <unordered_map>

// curses.h is only included by the .cpp: its macros (OK, ERR, move...) would
// leak into every file including this header.
struct screen; // ncurses' SCREEN

namespace client::gfx {

/**
 * @brief IGraphics backend drawing in a terminal with ncurses (Linux only).
 *
 * The terminal is seen as a screen of CellWidth x CellHeight pixels per
 * character, so the game code keeps its pixel coordinates. Shapes fill the
 * cells they cover, text is written cell by cell, colors are rounded to the
 * terminal's palette and textures are not supported (loadTexture() returns
 * no texture).
 *
 * A terminal only sends key presses and their auto-repeat, never releases:
 * a key counts as held for a short time after each press, and only one key
 * can be held at once. q or Ctrl+C closes the client.
 *
 * While it runs, stderr (the logs) is redirected to r-type_client.log in the
 * current directory, so the logs do not draw over the game.
 */
class NcursesGraphics final : public IGraphics {
  public:
    static constexpr float CellWidth = 8.0F;
    static constexpr float CellHeight = 16.0F;

    // Throws if the standard input or output is not a terminal.
    explicit NcursesGraphics(const WindowConfig &config);
    ~NcursesGraphics() override;

    NcursesGraphics(const NcursesGraphics &) = delete;
    NcursesGraphics &operator=(const NcursesGraphics &) = delete;
    NcursesGraphics(NcursesGraphics &&) = delete;
    NcursesGraphics &operator=(NcursesGraphics &&) = delete;

    [[nodiscard]] bool shouldClose() override;
    [[nodiscard]] engine::Vec2f screenSize() const override;

    void beginFrame(Color clear) override;
    void endFrame() override;
    void beginWorld(float zoom) override;
    void endWorld() override;

    [[nodiscard]] Texture loadTexture(const std::string &path) override;
    void unloadTexture(Texture texture) override;

    void drawRectangle(Rect rect, Color color) override;
    void drawTexture(Texture texture, Rect source, Rect dest,
                     Color tint) override;
    void drawText(const std::string &text, engine::Vec2f position, int fontSize,
                  Color color) override;

    [[nodiscard]] bool isKeyDown(Key key) const override;

  private:
    using Clock = std::chrono::steady_clock;
    static constexpr std::size_t KeyCount = 5;

    /** @brief Records a press of key, the start of a hold or a repeat. */
    void press(Key key, Clock::time_point now);
    /** @brief The color pair for these colors, created on first use; 0 (the
     * terminal's default colors) when the terminal has no free pair. */
    [[nodiscard]] int colorPair(short foreground, short background);

    screen *m_screen = nullptr;
    int m_savedStderr = -1;
    bool m_colors = false;
    bool m_vsync = true;
    bool m_closeRequested = false;
    float m_zoom = 1.0F;
    short m_clear = 0;
    std::unordered_map<std::uint32_t, int> m_pairs;
    std::array<Clock::time_point, KeyCount> m_lastPress{};
    std::array<Clock::time_point, KeyCount> m_heldUntil{};
    Clock::time_point m_nextFrame{};
};

} // namespace client::gfx

#endif // CLIENT_GRAPHICS_NCURSESGRAPHICS_HPP
