/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#include "Game.hpp"
#include "common/Playfield.hpp"
#include "components/Background.hpp"
#include "systems/BackgroundRenderSystem.hpp"
#include "systems/BackgroundScrollSystem.hpp"
#include "systems/HudSystem.hpp"
#include "systems/InputSystem.hpp"
#include "systems/RenderSystem.hpp"

#include <algorithm>
#include <chrono>
#include <string>

namespace client {

namespace {

constexpr int windowWidth = 1280;
constexpr int windowHeight = 720;

constexpr const char *backgroundPath = "assets/bg.jpg";
constexpr float backgroundSpeed = 120.0F;

constexpr float statusX = 20.0F;
constexpr float statusBottomMargin = 50.0F;
constexpr int statusFontSize = 30;

} // namespace

Game::Game(const std::string &host, std::uint16_t port)
    : m_network(host, port), m_graphics(gfx::createGraphics({
                                 .width = windowWidth,
                                 .height = windowHeight,
                                 .title = "R-Type",
                             }))
{
    spawnBackground();
}

Game::~Game() { m_graphics->unloadTexture(m_backgroundTexture); }

void Game::run()
{
    using Clock = std::chrono::steady_clock;
    constexpr engine::Duration maxFrameTime{0.25};

    Clock::time_point previous = Clock::now();

    while (!m_graphics->shouldClose()) {
        const Clock::time_point now = Clock::now();
        const engine::Duration dt =
            std::min(engine::Duration{now - previous}, maxFrameTime);
        previous = now;

        update(dt);
        render();
    }
}

void Game::update(engine::Duration dt)
{
    // Later: interpolation, animation.
    backgroundScrollSystem(m_registry, dt);
    const auto now = std::chrono::steady_clock::now();
    networkSyncSystem(m_registry, m_network.update(now), m_sync);
    m_network.sendInput(inputSystem(*m_graphics), now);

    m_registry.flush();
}

void Game::render()
{
    // The world is in playfield units: scale it to fit the window.
    const engine::Vec2f screen = m_graphics->screenSize();
    const float zoom = std::min(screen.x / rtype::PlayfieldWidth,
                                screen.y / rtype::PlayfieldHeight);

    m_graphics->beginFrame(gfx::colors::Black);
    backgroundRenderSystem(m_registry, *m_graphics);
    m_graphics->beginWorld(zoom);
    renderSystem(m_registry, *m_graphics);
    healthBarSystem(m_registry, *m_graphics);
    m_graphics->endWorld();
    hudSystem(m_sync, *m_graphics);
    drawStatus();
    m_graphics->endFrame();
}

void Game::spawnBackground()
{
    m_backgroundTexture = m_graphics->loadTexture(backgroundPath);

    const engine::Entity background = m_registry.create();
    m_registry.emplace<Background>(
        background,
        Background{.texture = m_backgroundTexture, .speed = backgroundSpeed});
}

void Game::drawStatus() const
{
    const char *text = nullptr;
    switch (m_network.state()) {
    case NetworkClient::State::Connecting:
        text = "Connecting...";
        break;
    case NetworkClient::State::Connected:
        return;
    case NetworkClient::State::Rejected:
        text = "Connection refused by the server";
        break;
    case NetworkClient::State::Lost:
        text = "Connection to the server lost";
        break;
    case NetworkClient::State::Closed:
        text = "Disconnected by the server";
        break;
    }
    m_graphics->drawText(
        text,
        {.x = statusX, .y = m_graphics->screenSize().y - statusBottomMargin},
        statusFontSize, gfx::colors::OffWhite);
}

} // namespace client
