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

/**
 * @brief Converts a color to Raylib's type.
 * @param color The color.
 * @return The same color.
 **/
::Color toRaylib(Color color)
{
    return {.r = color.r, .g = color.g, .b = color.b, .a = color.a};
}

/**
 * @brief Converts a rectangle to Raylib's type.
 * @param rect The rectangle.
 * @return The same rectangle.
 **/
Rectangle toRaylib(Rect rect)
{
    return {
        .x = rect.x, .y = rect.y, .width = rect.width, .height = rect.height};
}

/**
 * @brief Converts a key to Raylib's type.
 * @param key The key.
 * @return The same key.
 **/
KeyboardKey toRaylib(Key key)
{
    const auto index = static_cast<int>(key);
    if (key >= Key::A && key <= Key::Z) {
        return static_cast<KeyboardKey>(KEY_A + index -
                                        static_cast<int>(Key::A));
    }
    if (key >= Key::Num0 && key <= Key::Num9) {
        return static_cast<KeyboardKey>(KEY_ZERO + index -
                                        static_cast<int>(Key::Num0));
    }
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
    case Key::Enter:
        return KEY_ENTER;
    case Key::Tab:
        return KEY_TAB;
    case Key::Escape:
        return KEY_ESCAPE;
    case Key::LeftShift:
        return KEY_LEFT_SHIFT;
    case Key::RightShift:
        return KEY_RIGHT_SHIFT;
    case Key::LeftControl:
        return KEY_LEFT_CONTROL;
    case Key::RightControl:
        return KEY_RIGHT_CONTROL;
    case Key::LeftAlt:
        return KEY_LEFT_ALT;
    case Key::RightAlt:
        return KEY_RIGHT_ALT;
    default:
        return KEY_NULL;
    }
}

/** @brief The gamepad read: the first one plugged in. */
constexpr int Gamepad = 0;
/** @brief How far a stick must be pushed to count as a direction, from 0 to 1.
 */
constexpr float StickThreshold = 0.5F;

/**
 * @brief Converts a button of the gamepad to Raylib's type.
 * @param key The button, not a direction of the stick.
 * @return The same button, or GAMEPAD_BUTTON_UNKNOWN.
 **/
GamepadButton toRaylibButton(Key key)
{
    switch (key) {
    case Key::PadUp:
        return GAMEPAD_BUTTON_LEFT_FACE_UP;
    case Key::PadDown:
        return GAMEPAD_BUTTON_LEFT_FACE_DOWN;
    case Key::PadLeft:
        return GAMEPAD_BUTTON_LEFT_FACE_LEFT;
    case Key::PadRight:
        return GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
    case Key::PadA:
        return GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
    case Key::PadB:
        return GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;
    case Key::PadLB:
        return GAMEPAD_BUTTON_LEFT_TRIGGER_1;
    case Key::PadRB:
        return GAMEPAD_BUTTON_RIGHT_TRIGGER_1;
    case Key::PadLT:
        return GAMEPAD_BUTTON_LEFT_TRIGGER_2;
    case Key::PadRT:
        return GAMEPAD_BUTTON_RIGHT_TRIGGER_2;
    case Key::PadStart:
        return GAMEPAD_BUTTON_MIDDLE_RIGHT;
    case Key::PadSelect:
        return GAMEPAD_BUTTON_MIDDLE_LEFT;
    default:
        return GAMEPAD_BUTTON_UNKNOWN;
    }
}

/**
 * @brief Tells whether a button of the gamepad, or a direction of its left
 * stick, is held.
 * @param key A key for which isPadKey() is true.
 * @return `false` if no gamepad is plugged in.
 **/
bool isPadDown(Key key)
{
    if (!IsGamepadAvailable(Gamepad)) {
        return false;
    }
    const float x = GetGamepadAxisMovement(Gamepad, GAMEPAD_AXIS_LEFT_X);
    const float y = GetGamepadAxisMovement(Gamepad, GAMEPAD_AXIS_LEFT_Y);
    switch (key) {
    case Key::PadStickUp:
        return y < -StickThreshold;
    case Key::PadStickDown:
        return y > StickThreshold;
    case Key::PadStickLeft:
        return x < -StickThreshold;
    case Key::PadStickRight:
        return x > StickThreshold;
    default:
        return IsGamepadButtonDown(Gamepad, toRaylibButton(key));
    }
}

} // namespace

std::unique_ptr<IGraphics> createGraphics(const WindowConfig &config)
{
    return std::make_unique<RaylibGraphics>(config);
}

RaylibGraphics::RaylibGraphics(const WindowConfig &config)
    : m_chosenWidth(config.width), m_chosenHeight(config.height)
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
    // Escape goes back in the menus: the game decides when to quit.
    SetExitKey(KEY_NULL);
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

void RaylibGraphics::applyVideo(const VideoConfig &video)
{
    if (IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE) != video.fullscreen) {
        ToggleBorderlessWindowed();
    }
    const bool sizeChanged =
        video.width != m_chosenWidth || video.height != m_chosenHeight;
    if (!video.fullscreen && sizeChanged) {
        const int monitor = GetCurrentMonitor();
        SetWindowSize(video.width, video.height);
        SetWindowPosition(
            std::max((GetMonitorWidth(monitor) - video.width) / 2, 0),
            std::max((GetMonitorHeight(monitor) - video.height) / 2, 0));
        m_chosenWidth = video.width;
        m_chosenHeight = video.height;
    }
    if (video.vsync) {
        SetWindowState(FLAG_VSYNC_HINT);
    } else {
        ClearWindowState(FLAG_VSYNC_HINT);
    }
    SetTargetFPS(video.frameLimit);
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

Texture RaylibGraphics::loadTexture(const std::string &path,
                                    TextureFilter filter)
{
    const std::string fullPath = std::string{GetApplicationDirectory()} + path;
    if (!FileExists(fullPath.c_str())) {
        return {};
    }
    const Texture2D texture = LoadTexture(fullPath.c_str());
    if (texture.id == 0) {
        return {};
    }
    SetTextureFilter(texture, filter == TextureFilter::Smooth
                                  ? TEXTURE_FILTER_BILINEAR
                                  : TEXTURE_FILTER_POINT);

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

std::u32string RaylibGraphics::takeTextInput()
{
    std::u32string text;
    for (int character = GetCharPressed(); character != 0;
         character = GetCharPressed()) {
        text.push_back(static_cast<char32_t>(character));
    }
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        text.push_back(TextBackspace);
    }
    return text;
}

bool RaylibGraphics::isKeyDown(Key key) const
{
    if (isPadKey(key)) {
        return isPadDown(key);
    }
    return IsKeyDown(toRaylib(key));
}

engine::Vec2f RaylibGraphics::mousePosition() const
{
    const Vector2 position = GetMousePosition();
    return {.x = position.x, .y = position.y};
}

bool RaylibGraphics::isMousePressed() const
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

} // namespace client::gfx
