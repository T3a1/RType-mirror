/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SfmlGraphics
*/

#include "SfmlGraphics.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <array>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>

namespace client::gfx {

namespace {

constexpr const char *fontPath = "assets/fonts/DejaVuSans.ttf";

/**
 * @brief The directory holding the running executable, like Raylib's
 * GetApplicationDirectory().
 * @return The directory, or an empty path (the current directory) if it
 * cannot be found.
 */
std::filesystem::path executableDirectory()
{
#ifdef _WIN32
    std::array<wchar_t, MAX_PATH> buffer{};
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(),
                                            static_cast<DWORD>(buffer.size()));
    if (length == 0 || length == buffer.size()) {
        return {};
    }
    return std::filesystem::path{buffer.data()}.parent_path();
#else
    std::error_code error;
    const std::filesystem::path executable =
        std::filesystem::read_symlink("/proc/self/exe", error);
    return error ? std::filesystem::path{} : executable.parent_path();
#endif
}

sf::Color toSfml(Color color) { return {color.r, color.g, color.b, color.a}; }

sf::Keyboard::Key toSfml(Key key)
{
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
    }
    return sf::Keyboard::Key::Unknown;
}

} // namespace

std::unique_ptr<IGraphics> createGraphics(const WindowConfig &config)
{
    return std::make_unique<SfmlGraphics>(config);
}

SfmlGraphics::SfmlGraphics(const WindowConfig &config)
    : m_directory(executableDirectory())
{
    // Same as Raylib's LOG_NONE: a missing file is handled, not printed.
    sf::err().rdbuf(nullptr);

    const std::uint32_t style = config.resizable
                                    ? sf::Style::Default
                                    : (sf::Style::Titlebar | sf::Style::Close);
    m_window.create(sf::VideoMode({static_cast<unsigned int>(config.width),
                                   static_cast<unsigned int>(config.height)}),
                    config.title, style);
    if (!m_window.isOpen()) {
        throw std::runtime_error("cannot open the window");
    }
    m_window.setVerticalSyncEnabled(config.vsync);
    m_hasFont = m_font.openFromFile(m_directory / fontPath);
}

bool SfmlGraphics::shouldClose()
{
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_closeRequested = true;
        }
    }
    return m_closeRequested;
}

engine::Vec2f SfmlGraphics::screenSize() const
{
    const sf::Vector2u size = m_window.getSize();
    return {.x = static_cast<float>(size.x), .y = static_cast<float>(size.y)};
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

Texture SfmlGraphics::loadTexture(const std::string &path)
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
    texture.setSmooth(true);

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

bool SfmlGraphics::isKeyDown(Key key) const
{
    // SFML reads the global keyboard state: ignore it when unfocused.
    return m_window.hasFocus() && sf::Keyboard::isKeyPressed(toSfml(key));
}

} // namespace client::gfx
