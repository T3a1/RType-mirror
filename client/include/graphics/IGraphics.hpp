/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** IGraphics
*/

#ifndef CLIENT_GRAPHICS_IGRAPHICS_HPP
#define CLIENT_GRAPHICS_IGRAPHICS_HPP

#include "engine/Vec2.hpp"
#include "graphics/Types.hpp"

#include <memory>
#include <string>

namespace client::gfx {

/**
 * @brief The window, drawing and keyboard, independent of the graphics
 * library.
 *
 * The client only talks to this interface. Each backend (Raylib today)
 * implements it in client/src/graphics/<backend>/, the only place that
 * includes the library's headers; CMake picks one with
 * RTYPE_GRAPHICS_BACKEND.
 *
 * A frame is drawn between beginFrame() and endFrame(). Draw calls use screen
 * pixels, or world units scaled by a zoom between beginWorld() and
 * endWorld().
 */
class IGraphics {
  public:
    IGraphics() = default;
    virtual ~IGraphics() = default;

    IGraphics(const IGraphics &) = delete;
    IGraphics &operator=(const IGraphics &) = delete;
    IGraphics(IGraphics &&) = delete;
    IGraphics &operator=(IGraphics &&) = delete;

    /** @brief Processes the window events, true once the user closed it. */
    [[nodiscard]] virtual bool shouldClose() = 0;
    /** @brief The size of the window's drawable area, in pixels. */
    [[nodiscard]] virtual engine::Vec2f screenSize() const = 0;

    /** @brief Starts a frame and fills the window with a color. */
    virtual void beginFrame(Color clear) = 0;
    /** @brief Shows the frame, waiting for V-Sync if enabled. */
    virtual void endFrame() = 0;
    /** @brief Until endWorld(), coordinates are multiplied by zoom. */
    virtual void beginWorld(float zoom) = 0;
    virtual void endWorld() = 0;

    /**
     * @brief Loads an image file with bilinear filtering.
     * @param path Path relative to the directory of the executable.
     * @return The texture, with id 0 and a zero size if loading failed.
     */
    [[nodiscard]] virtual Texture loadTexture(const std::string &path) = 0;
    /** @brief Frees a texture, does nothing for id 0 or an unknown id. */
    virtual void unloadTexture(Texture texture) = 0;

    virtual void drawRectangle(Rect rect, Color color) = 0;
    /** @brief Draws the source part of a texture stretched over dest. */
    virtual void drawTexture(Texture texture, Rect source, Rect dest,
                             Color tint) = 0;
    /** @brief Draws text with the default font, position = top-left. */
    virtual void drawText(const std::string &text, engine::Vec2f position,
                          int fontSize, Color color) = 0;

    [[nodiscard]] virtual bool isKeyDown(Key key) const = 0;
};

/** @brief Settings of the window opened by createGraphics(). */
struct WindowConfig {
    int width = 0;
    int height = 0;
    std::string title;
    bool vsync = true;
    bool resizable = true;
};

/**
 * @brief Opens the window with the backend chosen at build time.
 *
 * Defined by the backend's source file. Throws if the window cannot be
 * opened.
 */
[[nodiscard]] std::unique_ptr<IGraphics>
createGraphics(const WindowConfig &config);

} // namespace client::gfx

#endif // CLIENT_GRAPHICS_IGRAPHICS_HPP
