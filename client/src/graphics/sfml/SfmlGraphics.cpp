/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SfmlGraphics
*/

#include "SfmlGraphics.hpp"
#include "ExecutableDirectory.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace client::gfx {

namespace {

/** @brief The font file, relative to the executable: SFML has no default font.
 */
constexpr const char *fontPath = "assets/fonts/DejaVuSans.ttf";

/**
 * @brief Converts a color to SFML's type.
 * @param color The color.
 * @return The same color.
 **/
sf::Color toSfml(Color color) { return {color.r, color.g, color.b, color.a}; }

/**
 * @brief Converts a key to SFML's type.
 * @param key The key.
 * @return The same key.
 **/
sf::Keyboard::Key toSfml(Key key)
{
    const auto index = static_cast<int>(key);
    if (key >= Key::A && key <= Key::Z) {
        return static_cast<sf::Keyboard::Key>(
            static_cast<int>(sf::Keyboard::Key::A) + index -
            static_cast<int>(Key::A));
    }
    if (key >= Key::Num0 && key <= Key::Num9) {
        return static_cast<sf::Keyboard::Key>(
            static_cast<int>(sf::Keyboard::Key::Num0) + index -
            static_cast<int>(Key::Num0));
    }
    switch (key) {
    case Key::Up:
        return sf::Keyboard::Key::Up;
    case Key::Down:
        return sf::Keyboard::Key::Down;
    case Key::Left:
        return sf::Keyboard::Key::Left;
    case Key::Right:
        return sf::Keyboard::Key::Right;
    case Key::Space:
        return sf::Keyboard::Key::Space;
    case Key::Enter:
        return sf::Keyboard::Key::Enter;
    case Key::Tab:
        return sf::Keyboard::Key::Tab;
    case Key::Escape:
        return sf::Keyboard::Key::Escape;
    case Key::LeftShift:
        return sf::Keyboard::Key::LShift;
    case Key::RightShift:
        return sf::Keyboard::Key::RShift;
    case Key::LeftControl:
        return sf::Keyboard::Key::LControl;
    case Key::RightControl:
        return sf::Keyboard::Key::RControl;
    case Key::LeftAlt:
        return sf::Keyboard::Key::LAlt;
    case Key::RightAlt:
        return sf::Keyboard::Key::RAlt;
    default:
        return sf::Keyboard::Key::Unknown;
    }
}

/** @brief The gamepad read: the first one plugged in. */
constexpr unsigned int Gamepad = 0;
/** @brief How far a stick, a trigger or the D-pad must be pushed to count, out
 * of SFML's 100. */
constexpr float AxisThreshold = 50.0F;
/** @brief The USB vendor of Sony, whose gamepads number their middle buttons
 * differently. */
constexpr unsigned int SonyVendor = 0x054C;

/**
 * @struct PadButtons
 * @brief The numbers SFML gives to some buttons of a gamepad. They follow the
 * driver: these are the ones of Linux, the same for Xbox and PlayStation
 * gamepads except for the middle buttons.
 * @param a The bottom face button.
 * @param b The right face button.
 * @param lb The left shoulder button.
 * @param rb The right shoulder button.
 * @param start The right middle button.
 * @param select The left middle button.
 **/
struct PadButtons {
    unsigned int a = 0;
    unsigned int b = 1;
    unsigned int lb = 4;
    unsigned int rb = 5;
    unsigned int start = 7;
    unsigned int select = 6;
};

/**
 * @brief Returns the numbers of the buttons of the plugged gamepad.
 * @return Those of a PlayStation gamepad, or of an Xbox one otherwise.
 **/
PadButtons padButtons()
{
    PadButtons buttons;
    if (sf::Joystick::getIdentification(Gamepad).vendorId == SonyVendor) {
        buttons.start = 9;
        buttons.select = 8;
    }
    return buttons;
}

/**
 * @brief Returns the position of an axis of the gamepad.
 * @param axis The axis.
 * @return Its position from -100 to 100, or 0 if the gamepad has no such axis.
 **/
float axis(sf::Joystick::Axis axis)
{
    return sf::Joystick::hasAxis(Gamepad, axis)
               ? sf::Joystick::getAxisPosition(Gamepad, axis)
               : 0.0F;
}

/**
 * @brief Tells whether a button of the gamepad, or a direction of its left
 * stick or D-pad, is held.
 * @param key A key for which isPadKey() is true.
 * @return `false` if no gamepad is plugged in.
 **/
bool isPadDown(Key key)
{
    using Axis = sf::Joystick::Axis;
    if (!sf::Joystick::isConnected(Gamepad)) {
        return false;
    }
    const PadButtons buttons = padButtons();
    const auto pressed = [](unsigned int button) {
        return sf::Joystick::isButtonPressed(Gamepad, button);
    };
    // A stick is negative up and left. A trigger goes from -100, released, to
    // 100. SFML gives the D-pad as the PovX and PovY axes, positive up.
    switch (key) {
    case Key::PadUp:
        return axis(Axis::PovY) > AxisThreshold;
    case Key::PadDown:
        return axis(Axis::PovY) < -AxisThreshold;
    case Key::PadLeft:
        return axis(Axis::PovX) < -AxisThreshold;
    case Key::PadRight:
        return axis(Axis::PovX) > AxisThreshold;
    case Key::PadStickUp:
        return axis(Axis::Y) < -AxisThreshold;
    case Key::PadStickDown:
        return axis(Axis::Y) > AxisThreshold;
    case Key::PadStickLeft:
        return axis(Axis::X) < -AxisThreshold;
    case Key::PadStickRight:
        return axis(Axis::X) > AxisThreshold;
    case Key::PadA:
        return pressed(buttons.a);
    case Key::PadB:
        return pressed(buttons.b);
    case Key::PadLB:
        return pressed(buttons.lb);
    case Key::PadRB:
        return pressed(buttons.rb);
    case Key::PadLT:
        return axis(Axis::Z) > 0.0F;
    case Key::PadRT:
        return axis(Axis::R) > 0.0F;
    case Key::PadStart:
        return pressed(buttons.start);
    case Key::PadSelect:
        return pressed(buttons.select);
    default:
        return false;
    }
}

} // namespace

std::unique_ptr<IGraphics> createGraphics(const WindowConfig &config)
{
    return std::make_unique<SfmlGraphics>(config);
}

SfmlGraphics::SfmlGraphics(const WindowConfig &config)
    : m_directory(executableDirectory()), m_title(config.title),
      m_style(config.resizable ? sf::Style::Default
                               : (sf::Style::Titlebar | sf::Style::Close)),
      m_chosenSize(static_cast<unsigned int>(config.width),
                   static_cast<unsigned int>(config.height))
{
    // Same as Raylib's LOG_NONE: a missing file is handled, not printed.
    sf::err().rdbuf(nullptr);

    m_window.create(sf::VideoMode(m_chosenSize), m_title, m_style);
    if (!m_window.isOpen()) {
        throw std::runtime_error("cannot open the window");
    }
    m_window.setVerticalSyncEnabled(config.vsync);
    m_hasFont = m_font.openFromFile(m_directory / fontPath);
}

bool SfmlGraphics::shouldClose()
{
    m_mousePressed = false;
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_closeRequested = true;
        }
        if (const auto *text = event->getIf<sf::Event::TextEntered>()) {
            m_typed.push_back(text->unicode);
        }
        const auto *mouse = event->getIf<sf::Event::MouseButtonPressed>();
        if (mouse != nullptr && mouse->button == sf::Mouse::Button::Left) {
            m_mousePressed = true;
        }
    }
    return m_closeRequested;
}

engine::Vec2f SfmlGraphics::screenSize() const
{
    const sf::Vector2u size = m_window.getSize();
    return {.x = static_cast<float>(size.x), .y = static_cast<float>(size.y)};
}

void SfmlGraphics::applyVideo(const VideoConfig &video)
{
    const sf::Vector2u size(static_cast<unsigned int>(video.width),
                            static_cast<unsigned int>(video.height));
    if (video.fullscreen != m_fullscreen) {
        // SFML only changes this by creating the window again; the textures
        // are kept.
        m_window.create(video.fullscreen ? sf::VideoMode::getDesktopMode()
                                         : sf::VideoMode(size),
                        m_title, m_style,
                        video.fullscreen ? sf::State::Fullscreen
                                         : sf::State::Windowed);
        m_fullscreen = video.fullscreen;
    } else if (!video.fullscreen && size != m_chosenSize) {
        const sf::Vector2u desktop = sf::VideoMode::getDesktopMode().size;
        m_window.setSize(size);
        m_window.setPosition(
            {std::max((static_cast<int>(desktop.x) - video.width) / 2, 0),
             std::max((static_cast<int>(desktop.y) - video.height) / 2, 0)});
    }
    m_chosenSize = size;
    m_window.setVerticalSyncEnabled(video.vsync);
    m_window.setFramerateLimit(static_cast<unsigned int>(video.frameLimit));
}

void SfmlGraphics::beginFrame(Color clear)
{
    useScreenView();
    m_window.clear(toSfml(clear));
}

void SfmlGraphics::endFrame() { m_window.display(); }

void SfmlGraphics::beginWorld(float zoom)
{
    if (zoom <= 0.0F) {
        return;
    }
    const engine::Vec2f screen = screenSize();
    m_window.setView(sf::View(
        sf::FloatRect({0.0F, 0.0F}, {screen.x / zoom, screen.y / zoom})));
}

void SfmlGraphics::endWorld() { useScreenView(); }

void SfmlGraphics::useScreenView()
{
    // Without this, SFML stretches the first view when the window is resized.
    const engine::Vec2f screen = screenSize();
    m_window.setView(
        sf::View(sf::FloatRect({0.0F, 0.0F}, {screen.x, screen.y})));
}

Texture SfmlGraphics::loadTexture(const std::string &path, TextureFilter filter)
{
    const std::filesystem::path fullPath = m_directory / path;
    std::error_code error;
    if (!std::filesystem::is_regular_file(fullPath, error)) {
        return {};
    }
    sf::Texture texture;
    if (!texture.loadFromFile(fullPath)) {
        return {};
    }
    texture.setSmooth(filter == TextureFilter::Smooth);

    const sf::Vector2u size = texture.getSize();
    const std::uint32_t id = m_nextTexture++;
    m_textures.emplace(id, std::move(texture));
    return {.id = id,
            .width = static_cast<int>(size.x),
            .height = static_cast<int>(size.y)};
}

void SfmlGraphics::unloadTexture(Texture texture)
{
    m_textures.erase(texture.id);
}

void SfmlGraphics::drawRectangle(Rect rect, Color color)
{
    sf::RectangleShape shape({rect.width, rect.height});
    shape.setPosition({rect.x, rect.y});
    shape.setFillColor(toSfml(color));
    m_window.draw(shape);
}

void SfmlGraphics::drawTexture(Texture texture, Rect source, Rect dest,
                               Color tint)
{
    const auto it = m_textures.find(texture.id);
    if (it == m_textures.end() || source.width <= 0.0F ||
        source.height <= 0.0F) {
        return;
    }
    sf::Sprite sprite(it->second);
    sprite.setTextureRect(sf::IntRect(
        {static_cast<int>(source.x), static_cast<int>(source.y)},
        {static_cast<int>(source.width), static_cast<int>(source.height)}));
    sprite.setPosition({dest.x, dest.y});
    sprite.setScale({dest.width / source.width, dest.height / source.height});
    sprite.setColor(toSfml(tint));
    m_window.draw(sprite);
}

void SfmlGraphics::drawText(const std::string &text, engine::Vec2f position,
                            int fontSize, Color color)
{
    if (!m_hasFont || fontSize <= 0) {
        return;
    }
    sf::Text label(m_font, text, static_cast<unsigned int>(fontSize));
    label.setPosition({position.x, position.y});
    label.setFillColor(toSfml(color));
    m_window.draw(label);
}

std::u32string SfmlGraphics::takeTextInput()
{
    return std::exchange(m_typed, {});
}

bool SfmlGraphics::isKeyDown(Key key) const
{
    // SFML reads the global keyboard and gamepad state: ignore it when
    // unfocused.
    if (!m_window.hasFocus()) {
        return false;
    }
    if (isPadKey(key)) {
        return isPadDown(key);
    }
    return sf::Keyboard::isKeyPressed(toSfml(key));
}

engine::Vec2f SfmlGraphics::mousePosition() const
{
    const sf::Vector2i position = sf::Mouse::getPosition(m_window);
    return {.x = static_cast<float>(position.x),
            .y = static_cast<float>(position.y)};
}

bool SfmlGraphics::isMousePressed() const { return m_mousePressed; }

} // namespace client::gfx
