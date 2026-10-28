/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** RaylibGraphics
*/

#include "RaylibGraphics.hpp"

#include <stdexcept>
#include <string>

namespace client::gfx {

namespace {

::Color toRaylib(Color color)
{
    return {.r = color.r, .g = color.g, .b = color.b, .a = color.a};
}

Rectangle toRaylib(Rect rect)
{
    return {
        .x = rect.x, .y = rect.y, .width = rect.width, .height = rect.height};
}

KeyboardKey toRaylib(Key key)
{
    switch (key) {
    case Key::Up:
        return KEY_UP;
    case Key::Down:
        return KEY_DOWN;
    case Key::Left:
        return KEY_LEFT;
    case Key::Right:
        return KEY_RIGHT;
    case Key::Space:
        return KEY_SPACE;
    }
    return KEY_NULL;
}

} // namespace

std::unique_ptr<IGraphics> createGraphics(const WindowConfig &config)
{
    return std::make_unique<RaylibGraphics>(config);
}

RaylibGraphics::RaylibGraphics(const WindowConfig &config)
{
    SetTraceLogLevel(LOG_NONE);
    unsigned int flags = 0;
    if (config.vsync) {
        flags |= FLAG_VSYNC_HINT;
    }
    if (config.resizable) {
        flags |= FLAG_WINDOW_RESIZABLE;
    }
    SetConfigFlags(flags);
    InitWindow(config.width, config.height, config.title.c_str());
    if (!IsWindowReady()) {
        throw std::runtime_error("cannot open the window");
    }
}

RaylibGraphics::~RaylibGraphics()
{
    for (const auto &[id, texture] : m_textures) {
        UnloadTexture(texture);
    }
    CloseWindow();
}

bool RaylibGraphics::shouldClose() { return WindowShouldClose(); }

engine::Vec2f RaylibGraphics::screenSize() const
{
    return {.x = static_cast<float>(GetScreenWidth()),
            .y = static_cast<float>(GetScreenHeight())};
}

void RaylibGraphics::beginFrame(Color clear)
{
    BeginDrawing();
    ClearBackground(toRaylib(clear));
}

void RaylibGraphics::endFrame() { EndDrawing(); }

void RaylibGraphics::beginWorld(float zoom)
{
    BeginMode2D(Camera2D{
        .offset = {.x = 0.0F, .y = 0.0F},
        .target = {.x = 0.0F, .y = 0.0F},
        .rotation = 0.0F,
        .zoom = zoom,
    });
}

void RaylibGraphics::endWorld() { EndMode2D(); }

Texture RaylibGraphics::loadTexture(const std::string &path)
{
    const std::string fullPath = std::string{GetApplicationDirectory()} + path;
    if (!FileExists(fullPath.c_str())) {
        return {};
    }
    const Texture2D texture = LoadTexture(fullPath.c_str());
    if (texture.id == 0) {
        return {};
    }
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);

    const std::uint32_t id = m_nextTexture++;
    m_textures.emplace(id, texture);
    return {.id = id, .width = texture.width, .height = texture.height};
}

void RaylibGraphics::unloadTexture(Texture texture)
{
    const auto it = m_textures.find(texture.id);
    if (it == m_textures.end()) {
        return;
    }
    UnloadTexture(it->second);
    m_textures.erase(it);
}

void RaylibGraphics::drawRectangle(Rect rect, Color color)
{
    DrawRectangleRec(toRaylib(rect), toRaylib(color));
}

void RaylibGraphics::drawTexture(Texture texture, Rect source, Rect dest,
                                 Color tint)
{
    const auto it = m_textures.find(texture.id);
    if (it == m_textures.end()) {
        return;
    }
    DrawTexturePro(it->second, toRaylib(source), toRaylib(dest),
                   Vector2{.x = 0.0F, .y = 0.0F}, 0.0F, toRaylib(tint));
}

void RaylibGraphics::drawText(const std::string &text, engine::Vec2f position,
                              int fontSize, Color color)
{
    DrawText(text.c_str(), static_cast<int>(position.x),
             static_cast<int>(position.y), fontSize, toRaylib(color));
}

bool RaylibGraphics::isKeyDown(Key key) const
{
    return IsKeyDown(toRaylib(key));
}

} // namespace client::gfx
