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

#include <raylib.h>

#include <algorithm>
#include <chrono>
#include <string>

namespace client {

namespace {

constexpr const char *backgroundPath = "assets/bg.jpg";
constexpr float backgroundSpeed = 120.0F;

} // namespace

Game::Game(const std::string &host, std::uint16_t port) : m_network(host, port)
{
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, "R-Type");
    spawnBackground();
}

Game::~Game()
{
    UnloadTexture(m_backgroundTexture);
    CloseWindow();
}

void Game::run()
{
    using Clock = std::chrono::steady_clock;
    constexpr engine::Duration maxFrameTime{0.25};

    Clock::time_point previous = Clock::now();

    while (!WindowShouldClose()) {
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
    m_network.sendInput(inputSystem(), now);

    m_registry.flush();
}

void Game::render()
{
    // The world is in playfield units: scale it to fit the window.
    const float zoom = std::min(
        static_cast<float>(GetScreenWidth()) / rtype::PlayfieldWidth,
        static_cast<float>(GetScreenHeight()) / rtype::PlayfieldHeight);

    BeginDrawing();
    ClearBackground(BLACK);
    backgroundRenderSystem(m_registry);
    BeginMode2D(Camera2D{
        .offset = {.x = 0.0F, .y = 0.0F},
        .target = {.x = 0.0F, .y = 0.0F},
        .rotation = 0.0F,
        .zoom = zoom,
    });
    renderSystem(m_registry);
    healthBarSystem(m_registry);
    EndMode2D();
    hudSystem(m_sync);
    drawStatus();
    EndDrawing();
}

void Game::spawnBackground()
{
    const std::string path =
        std::string{GetApplicationDirectory()} + backgroundPath;
    m_backgroundTexture = LoadTexture(path.c_str());
    SetTextureFilter(m_backgroundTexture, TEXTURE_FILTER_BILINEAR);

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
    DrawText(text, 20, GetScreenHeight() - 50, 30, RAYWHITE);
}

} // namespace client
