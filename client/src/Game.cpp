/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#include "Game.hpp"
#include "common/Playfield.hpp"
#include "systems/HudSystem.hpp"
#include "systems/InputSystem.hpp"
#include "systems/RenderSystem.hpp"

#include <raylib.h>

#include <algorithm>
#include <chrono>

namespace client {

namespace {

constexpr int windowWidth = 1280;
constexpr int windowHeight = 720;

} // namespace

Game::Game(const std::string &host, std::uint16_t port) : m_network(host, port)
{
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, "R-Type");
}

Game::~Game() { CloseWindow(); }

void Game::run()
{
    while (!WindowShouldClose()) {
        update();
        render();
    }
}

void Game::update()
{
    // Later: interpolation, animation, background scrolling.
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
