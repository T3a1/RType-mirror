/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#include "Game.hpp"
#include "common/components/Transform.hpp"
#include "components/Sprite.hpp"
#include "systems/RenderSystem.hpp"

#include <raylib.h>

#include <algorithm>
#include <chrono>

namespace client {

namespace {

constexpr int windowWidth = 1280;
constexpr int windowHeight = 720;

} // namespace

Game::Game()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, "R-Type");
    spawnTestEntities();
}

Game::~Game() { CloseWindow(); }

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

void Game::update(engine::Duration /*dt*/)
{
    // Later, in this order: input, network sync, interpolation, animation,
    // background scrolling.

    m_registry.flush();
}

void Game::render()
{
    BeginDrawing();
    ClearBackground(BLACK);
    renderSystem(m_registry);
    EndDrawing();
}

// Temporaire, plus gtard les Entities devront venir du serveur
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
