/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SfmlGraphics
*/

#ifndef CLIENT_GRAPHICS_SFMLGRAPHICS_HPP
#define CLIENT_GRAPHICS_SFMLGRAPHICS_HPP

#include "graphics/IGraphics.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Joystick.hpp>

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace client::gfx {

/**
 * @class SfmlGraphics
 * @brief IGraphics backend using SFML.
 *
 * SFML has no default font: drawText() uses assets/fonts/DejaVuSans.ttf and
 * draws nothing if it could not be loaded.
 *
 * The gamepad is the first device with enough buttons and a stick. SFML
 * numbers its buttons as the system does, so a layout is chosen for the system
 * and the vendor. With RTYPE_PAD_DEBUG set, the client logs the gamepad found
 * and every button and axis that changes, to check a layout.
 */
class SfmlGraphics final : public IGraphics {
  public:
    /**
     * @brief Opens the window.
     * @param config The settings of the window.
     * @throws std::runtime_error If the window cannot be opened.
     **/
    explicit SfmlGraphics(const WindowConfig &config);
    ~SfmlGraphics() override = default;

    SfmlGraphics(const SfmlGraphics &) = delete;
    SfmlGraphics &operator=(const SfmlGraphics &) = delete;
    SfmlGraphics(SfmlGraphics &&) = delete;
    SfmlGraphics &operator=(SfmlGraphics &&) = delete;

    [[nodiscard]] bool shouldClose() override;
    [[nodiscard]] engine::Vec2f screenSize() const override;
    void applyVideo(const VideoConfig &video) override;

    void beginFrame(Color clear) override;
    void endFrame() override;
    void beginWorld(float zoom) override;
    void endWorld() override;

    [[nodiscard]] Texture loadTexture(const std::string &path,
                                      TextureFilter filter) override;
    void unloadTexture(Texture texture) override;

    void drawRectangle(Rect rect, Color color) override;
    void drawTexture(Texture texture, Rect source, Rect dest,
                     Color tint) override;
    void drawText(const std::string &text, engine::Vec2f position, int fontSize,
                  Color color) override;

    [[nodiscard]] bool isKeyDown(Key key) const override;
    [[nodiscard]] engine::Vec2f mousePosition() const override;
    [[nodiscard]] bool isMousePressed() const override;
    [[nodiscard]] bool hasTextInput() const override { return true; }
    [[nodiscard]] std::u32string takeTextInput() override;

  private:
    /** @brief Draws in screen pixels, whatever the window's current size. */
    void useScreenView();
    /** @brief Finds the gamepad again, as one may be plugged in or out, and
     * logs it in debug mode. */
    void updateGamepad();
    /** @brief Logs the buttons and axes of a gamepad that changed. */
    void logGamepad(unsigned int gamepad);

    std::filesystem::path m_directory;
    sf::RenderWindow m_window;
    std::string m_title;
    std::uint32_t m_style = sf::Style::Default;
    sf::Vector2u m_chosenSize;
    bool m_fullscreen = false;
    sf::Font m_font;
    bool m_hasFont = false;
    std::u32string m_typed;
    bool m_closeRequested = false;
    bool m_mousePressed = false;
    std::unordered_map<std::uint32_t, sf::Texture> m_textures;
    std::uint32_t m_nextTexture = 1;
    std::optional<unsigned int> m_gamepad;
    unsigned int m_padVendor = 0;
    bool m_padDebug = false;
    std::array<bool, sf::Joystick::ButtonCount> m_padButtons{};
    std::array<int, sf::Joystick::AxisCount> m_padAxes{};
};

} // namespace client::gfx

#endif // CLIENT_GRAPHICS_SFMLGRAPHICS_HPP
