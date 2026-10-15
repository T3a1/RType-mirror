/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#include "Game.hpp"
#include "Window.hpp"
#include "common/components/InputState.hpp"
#include "common/components/Transform.hpp"
#include "common/components/Velocity.hpp"
#include "components/Background.hpp"
#include "components/Sprite.hpp"
#include "systems/BackgroundRenderSystem.hpp"
#include "systems/RenderSystem.hpp"

#include "common/systems/MovementSystem.hpp"
#include "common/systems/PlayerControlSystem.hpp"
#include "systems/BackgroundScrollSystem.hpp"
#include "systems/DebugSpawnSystem.hpp"
#include "systems/InputSystem.hpp"

#include <raylib.h>

#include <algorithm>
#include <chrono>
#include <string>

namespace client {

namespace {

constexpr const char *backgroundPath = "assets/bg.jpg";
constexpr float backgroundSpeed = 120.0F;

} // namespace

Game::Game()
{
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, "R-Type");
    spawnBackground();
    spawnTestEntities();
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
    // Later, in this order: input, network sync, interpolation, animation
    inputSystem(m_registry);
    debugSpawnSystem(m_registry, m_random);
    rtype::playerControlSystem(m_registry);
    rtype::movementSystem(m_registry, dt);
    backgroundScrollSystem(m_registry, dt);

    m_registry.flush();
}

void Game::render()
{
    BeginDrawing();
    ClearBackground(BLACK);
    backgroundRenderSystem(m_registry);
    renderSystem(m_registry);
    EndDrawing();
}

void Game::spawnBackground()
{
    const std::string path =
        std::string{GetApplicationDirectory()} + backgroundPath;
    m_backgroundTexture = LoadTexture(path.c_str());
    SetTextureFilter(m_backgroundTexture, TEXTURE_FILTER_BILINEAR);

    const engine::Entity background = m_registry.create();
    m_registry.emplace<Background>(background, Background{.texture = m_backgroundTexture, .speed = backgroundSpeed});
}

// Temporaire, plus tard les Entities devront venir du serveur
void Game::spawnTestEntities()
{
    const engine::Entity ship = m_registry.create();
    m_registry.emplace<rtype::Transform>(
        ship, rtype::Transform{.position = {.x = 200.0F, .y = 360.0F}});
    m_registry.emplace<Sprite>(
        ship, Sprite{
                  .size = {.x = 64.0F, .y = 32.0F},
                  .tint = {.r = 80, .g = 160, .b = 255, .a = 255},
                  .layer = 1,
              });
    m_registry.emplace<rtype::InputState>(ship, rtype::InputState{});
    m_registry.emplace<rtype::Velocity>(ship, rtype::Velocity{});

    const engine::Entity enemy = m_registry.create();
    m_registry.emplace<rtype::Transform>(
        enemy, rtype::Transform{.position = {.x = 1000.0F, .y = 300.0F}});
    m_registry.emplace<Sprite>(
        enemy, Sprite{
                   .size = {.x = 48.0F, .y = 48.0F},
                   .tint = {.r = 230, .g = 60, .b = 60, .a = 255},
                   .layer = 1,
               });
}

} // namespace client
