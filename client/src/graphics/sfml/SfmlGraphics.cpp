/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SfmlGraphics
*/

#include "SfmlGraphics.hpp"
#include "ExecutableDirectory.hpp"
#include "engine/Logger.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <ios>
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

/** @brief How far a stick, a trigger or the D-pad must be pushed to count, out
 * of SFML's 100. */
constexpr float AxisThreshold = 50.0F;
/** @brief The fewest buttons a device needs to be taken for a gamepad: the
 * motion sensors and the touchpad of a DualSense are devices too. */
constexpr unsigned int MinGamepadButtons = 6;
/** @brief The USB vendor of Sony. */
constexpr unsigned int SonyVendor = 0x054C;
/** @brief The name of each axis of SFML, in the order of sf::Joystick::Axis. */
constexpr std::array<const char *, sf::Joystick::AxisCount> AxisNames{
    "X", "Y", "Z", "R", "U", "V", "PovX", "PovY",
};

/**
 * @enum Triggers
 * @brief How a driver gives the two triggers of a gamepad.
 * @param SeparateAxes The Z axis for the left one and R for the right one,
 * from -100 released to 100.
 * @param Buttons Two buttons, lt and rt in PadLayout.
 * @param SharedAxis One Z axis for both, positive for the left one and
 * negative for the right one.
 **/
enum class Triggers : std::uint8_t { SeparateAxes, Buttons, SharedAxis };

/**
 * @struct PadLayout
 * @brief The numbers SFML gives to the buttons of a gamepad, which depend on
 * the system and the vendor. RTYPE_PAD_DEBUG=1 prints them to check a layout.
 * @param a The bottom face button: A, or Cross.
 * @param b The right face button: B, or Circle.
 * @param lb The left shoulder button.
 * @param rb The right shoulder button.
 * @param lt The left trigger, when triggers is Buttons.
 * @param rt The right trigger, when triggers is Buttons.
 * @param select The left middle button: Back, or Create.
 * @param start The right middle button: Start, or Options.
 * @param triggers How the triggers are read.
 * @param povUpPositive Whether up on the D-pad makes PovY positive.
 **/
struct PadLayout {
    unsigned int a;
    unsigned int b;
    unsigned int lb;
    unsigned int rb;
    unsigned int lt;
    unsigned int rt;
    unsigned int select;
    unsigned int start;
    Triggers triggers;
    bool povUpPositive;
};

// The driver of Linux puts the bottom button first for every vendor, and up on
// the D-pad makes PovY negative.
/** @brief An Xbox gamepad on Linux. */
constexpr PadLayout LinuxXbox{.a = 0,
                              .b = 1,
                              .lb = 4,
                              .rb = 5,
                              .lt = 6,
                              .rt = 7,
                              .select = 6,
                              .start = 7,
                              .triggers = Triggers::SeparateAxes,
                              .povUpPositive = false};
/** @brief A PlayStation gamepad on Linux. */
constexpr PadLayout LinuxSony{.a = 0,
                              .b = 1,
                              .lb = 4,
                              .rb = 5,
                              .lt = 6,
                              .rt = 7,
                              .select = 8,
                              .start = 9,
                              .triggers = Triggers::SeparateAxes,
                              .povUpPositive = false};
// Windows and macOS keep the order of the gamepad: Square, Cross, Circle,
// Triangle on a PlayStation one.
/** @brief A PlayStation gamepad on Windows or macOS. */
constexpr PadLayout HidSony{.a = 1,
                            .b = 2,
                            .lb = 4,
                            .rb = 5,
                            .lt = 6,
                            .rt = 7,
                            .select = 8,
                            .start = 9,
                            .triggers = Triggers::Buttons,
                            .povUpPositive = true};
/** @brief An Xbox gamepad on Windows or macOS. */
constexpr PadLayout HidXbox{.a = 0,
                            .b = 1,
                            .lb = 4,
                            .rb = 5,
                            .lt = 6,
                            .rt = 7,
                            .select = 6,
                            .start = 7,
                            .triggers = Triggers::SharedAxis,
                            .povUpPositive = true};

/**
 * @brief Returns the layout of a gamepad on this system.
 * @param vendorId The USB vendor of the gamepad.
 * @return The layout of a PlayStation gamepad for Sony, of an Xbox one
 * otherwise.
 **/
PadLayout padLayout(unsigned int vendorId)
{
    const bool sony = vendorId == SonyVendor;
#if defined(_WIN32) || defined(__APPLE__)
    return sony ? HidSony : HidXbox;
#else
    return sony ? LinuxSony : LinuxXbox;
#endif
}

/**
 * @brief Finds the gamepad to read.
 * @return The first connected device with enough buttons and a stick, if any.
 **/
std::optional<unsigned int> findGamepad()
{
    for (unsigned int i = 0; i < sf::Joystick::Count; ++i) {
        if (sf::Joystick::isConnected(i) &&
            sf::Joystick::getButtonCount(i) >= MinGamepadButtons &&
            sf::Joystick::hasAxis(i, sf::Joystick::Axis::X) &&
            sf::Joystick::hasAxis(i, sf::Joystick::Axis::Y)) {
            return i;
        }
    }
    return std::nullopt;
}

/**
 * @brief Returns the position of an axis of a gamepad.
 * @param gamepad The gamepad.
 * @param axis The axis.
 * @return Its position from -100 to 100, or 0 if the gamepad has no such axis.
 **/
float axisOf(unsigned int gamepad, sf::Joystick::Axis axis)
{
    return sf::Joystick::hasAxis(gamepad, axis)
               ? sf::Joystick::getAxisPosition(gamepad, axis)
               : 0.0F;
}

/**
 * @brief Tells whether a trigger of a gamepad is held.
 * @param gamepad The gamepad.
 * @param layout Its layout.
 * @param left Whether it is the left trigger.
 * @return `true` if it is pushed past the middle.
 **/
bool isTriggerDown(unsigned int gamepad, const PadLayout &layout, bool left)
{
    using Axis = sf::Joystick::Axis;
    switch (layout.triggers) {
    case Triggers::SeparateAxes:
        return axisOf(gamepad, left ? Axis::Z : Axis::R) > 0.0F;
    case Triggers::Buttons:
        return sf::Joystick::isButtonPressed(gamepad,
                                             left ? layout.lt : layout.rt);
    case Triggers::SharedAxis: {
        const float z = axisOf(gamepad, Axis::Z);
        return left ? z > AxisThreshold : z < -AxisThreshold;
    }
    }
    return false;
}

/**
 * @brief Tells whether a button of a gamepad, or a direction of its left
 * stick or D-pad, is held.
 * @param key A key for which isPadKey() is true.
 * @param gamepad The gamepad.
 * @param layout Its layout.
 * @return `true` if it is held.
 **/
bool isPadDown(Key key, unsigned int gamepad, const PadLayout &layout)
{
    using Axis = sf::Joystick::Axis;
    const auto pressed = [gamepad](unsigned int button) {
        return sf::Joystick::isButtonPressed(gamepad, button);
    };
    const float povUp = layout.povUpPositive ? axisOf(gamepad, Axis::PovY)
                                             : -axisOf(gamepad, Axis::PovY);
    // A stick is negative up and left.
    switch (key) {
    case Key::PadUp:
        return povUp > AxisThreshold;
    case Key::PadDown:
        return povUp < -AxisThreshold;
    case Key::PadLeft:
        return axisOf(gamepad, Axis::PovX) < -AxisThreshold;
    case Key::PadRight:
        return axisOf(gamepad, Axis::PovX) > AxisThreshold;
    case Key::PadStickUp:
        return axisOf(gamepad, Axis::Y) < -AxisThreshold;
    case Key::PadStickDown:
        return axisOf(gamepad, Axis::Y) > AxisThreshold;
    case Key::PadStickLeft:
        return axisOf(gamepad, Axis::X) < -AxisThreshold;
    case Key::PadStickRight:
        return axisOf(gamepad, Axis::X) > AxisThreshold;
    case Key::PadA:
        return pressed(layout.a);
    case Key::PadB:
        return pressed(layout.b);
    case Key::PadLB:
        return pressed(layout.lb);
    case Key::PadRB:
        return pressed(layout.rb);
    case Key::PadLT:
        return isTriggerDown(gamepad, layout, true);
    case Key::PadRT:
        return isTriggerDown(gamepad, layout, false);
    case Key::PadStart:
        return pressed(layout.start);
    case Key::PadSelect:
        return pressed(layout.select);
    default:
        return false;
    }
}

/**
 * @brief Returns in which third of its range an axis is.
 * @param position The position of the axis, from -100 to 100.
 * @return -1 past the threshold below, 1 past it above, 0 between.
 **/
int axisZone(float position)
{
    if (position > AxisThreshold) {
        return 1;
    }
    return position < -AxisThreshold ? -1 : 0;
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
    m_padDebug = std::getenv("RTYPE_PAD_DEBUG") != nullptr;
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
    updateGamepad();
    return m_closeRequested;
}

void SfmlGraphics::updateGamepad()
{
    const std::optional<unsigned int> found = findGamepad();
    if (found != m_gamepad) {
        m_gamepad = found;
        m_padVendor =
            found ? sf::Joystick::getIdentification(*found).vendorId : 0;
        m_padButtons.fill(false);
        m_padAxes.fill(0);
        if (m_padDebug && found) {
            const sf::Joystick::Identification id =
                sf::Joystick::getIdentification(*found);
            BB_INFO("[pad] device "
                    << *found << ": \"" << id.name.toAnsiString()
                    << "\", vendor " << std::hex << id.vendorId << ", product "
                    << id.productId << std::dec << ", "
                    << sf::Joystick::getButtonCount(*found) << " buttons");
        }
    }
    if (m_padDebug && m_gamepad) {
        logGamepad(*m_gamepad);
    }
}

void SfmlGraphics::logGamepad(unsigned int gamepad)
{
    const unsigned int buttons = std::min(sf::Joystick::getButtonCount(gamepad),
                                          sf::Joystick::ButtonCount);
    for (unsigned int i = 0; i < buttons; ++i) {
        const bool down = sf::Joystick::isButtonPressed(gamepad, i);
        if (down != m_padButtons.at(i)) {
            m_padButtons.at(i) = down;
            BB_INFO("[pad] button " << i << (down ? " pressed" : " released"));
        }
    }
    for (unsigned int i = 0; i < sf::Joystick::AxisCount; ++i) {
        const auto axis = static_cast<sf::Joystick::Axis>(i);
        if (!sf::Joystick::hasAxis(gamepad, axis)) {
            continue;
        }
        const float position = sf::Joystick::getAxisPosition(gamepad, axis);
        const int zone = axisZone(position);
        if (zone != m_padAxes.at(i)) {
            m_padAxes.at(i) = zone;
            BB_INFO("[pad] axis " << AxisNames.at(i) << " = " << position);
        }
    }
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
        return m_gamepad && isPadDown(key, *m_gamepad, padLayout(m_padVendor));
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
