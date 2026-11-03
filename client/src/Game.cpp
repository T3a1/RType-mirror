/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#include "Game.hpp"
#include "Window.hpp"
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
#include <utility>

namespace client {

namespace {

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
    registerSystems();
    registerRenderSystems();
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
        render(dt);
    }
}

/**
 * Registers the logic systems in the scheduler, stage by stage and in the
 * order they run within a stage.
 **/
void Game::registerSystems()
{
    using engine::Duration;
    using engine::Registry;
    using engine::Stage;

    m_scheduler.add(Stage::Input, "networkSync", [this](Registry &r, Duration) {
        networkSyncSystem(r, m_network.update(std::chrono::steady_clock::now()),
                          m_sync);
    });
    m_scheduler.add(Stage::Simulation, "backgroundScroll",
                    backgroundScrollSystem);
    m_scheduler.add(
        Stage::Simulation, "smoothing",
        [this](Registry &r, Duration dt) { smoothingSystem(r, m_sync, dt); });
    m_scheduler.add(Stage::Output, "sendInput", [this](Registry &, Duration) {
        m_network.sendInput(inputSystem(*m_graphics),
                            std::chrono::steady_clock::now());
    });
}

/**
 * Adds a drawing system to a layer. Within a layer, systems draw in the
 * order they were added.
 * @param layer The layer the system draws in.
 * @param name The name of the system, for logs and debugging.
 * @param system The system to run.
 **/
void Game::addRender(Layer layer, std::string name, engine::System system)
{
    m_layers.at(static_cast<std::size_t>(layer))
        .add(engine::Stage::Output, std::move(name), std::move(system));
}

/**
 * Registers the drawing systems in their layer, from back to front.
 **/
void Game::registerRenderSystems()
{
    using engine::Duration;
    using engine::Registry;

    addRender(Layer::Background, "background", [this](Registry &r, Duration) {
        backgroundRenderSystem(r, *m_graphics);
    });
    addRender(Layer::World, "sprites",
              [this](Registry &r, Duration) { renderSystem(r, *m_graphics); });
    addRender(Layer::World, "healthBars", [this](Registry &r, Duration) {
        healthBarSystem(r, *m_graphics);
    });
    addRender(Layer::Overlay, "hud",
              [this](Registry &, Duration) { hudSystem(m_sync, *m_graphics); });
    addRender(Layer::Overlay, "status",
              [this](Registry &, Duration) { drawStatus(); });
}

void Game::update(engine::Duration dt) { m_scheduler.run(m_registry, dt); }

void Game::render(engine::Duration dt)
{
    // The world is in playfield units: scale it to fit the window.
    const engine::Vec2f screen = m_graphics->screenSize();
    const float zoom = std::min(screen.x / rtype::PlayfieldWidth,
                                screen.y / rtype::PlayfieldHeight);
    const auto draw = [&](Layer layer) {
        m_layers.at(static_cast<std::size_t>(layer)).run(m_registry, dt);
    };

    m_graphics->beginFrame(gfx::colors::Black);
    draw(Layer::Background);
    m_graphics->beginWorld(zoom);
    draw(Layer::World);
    m_graphics->endWorld();
    draw(Layer::Overlay);
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
