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
 * @class IGraphics
 * @brief The window, drawing and keyboard, independent of the graphics
 * library.
 *
 * The client only talks to this interface. Each backend (Raylib today)
 * implements it in `client/src/graphics/<backend>/`, the only place that
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

    /**
     * @brief Processes the window events.
     * @return `true` once the user closed the window or pressed Escape.
     **/
    [[nodiscard]] virtual bool shouldClose() = 0;
    /**
     * @brief Returns the size of the window's drawable area.
     * @return Its width and height, in pixels.
     **/
    [[nodiscard]] virtual engine::Vec2f screenSize() const = 0;

    /**
     * @brief Starts a frame and fills the window with a color.
     * @param clear The color.
     **/
    virtual void beginFrame(Color clear) = 0;
    /**
     * @brief Shows the frame, waiting for V-Sync if enabled.
     **/
    virtual void endFrame() = 0;
    /**
     * @brief Switches to world coordinates: until endWorld(), coordinates are
     * multiplied by zoom.
     * @param zoom How many pixels one world unit covers.
     **/
    virtual void beginWorld(float zoom) = 0;
    /**
     * @brief Goes back to screen pixels after beginWorld().
     **/
    virtual void endWorld() = 0;

    /**
     * @brief Loads an image file.
     * @param path Path relative to the directory of the executable.
     * @param filter How the texture is sampled when it is scaled.
     * @return The texture, with id 0 and a zero size if loading failed.
     */
    [[nodiscard]] virtual Texture loadTexture(const std::string &path,
                                              TextureFilter filter) = 0;
    /**
     * @brief Frees a texture, does nothing for id 0 or an unknown id.
     * @param texture The texture.
     **/
    virtual void unloadTexture(Texture texture) = 0;

    /**
     * @brief Fills a rectangle with a color.
     * @param rect The rectangle, in screen pixels or world units.
     * @param color The color.
     **/
    virtual void drawRectangle(Rect rect, Color color) = 0;
    /**
     * @brief Draws a part of a texture stretched over a rectangle.
     * @param texture The texture.
     * @param source The part of it to draw, in texture pixels.
     * @param dest Where to draw it, in screen pixels or world units.
     * @param tint Multiplied with the pixels of the texture.
     **/
    virtual void drawTexture(Texture texture, Rect source, Rect dest,
                             Color tint) = 0;
    /**
     * @brief Draws text with the default font.
     * @param text The text.
     * @param position Its top-left corner, in screen pixels or world units.
     * @param fontSize Its height, in pixels or world units.
     * @param color Its color.
     **/
    virtual void drawText(const std::string &text, engine::Vec2f position,
                          int fontSize, Color color) = 0;

    /**
     * @brief Tells whether a key is held.
     * @param key The key.
     * @return `true` if it is held.
     **/
    [[nodiscard]] virtual bool isKeyDown(Key key) const = 0;
    /**
     * @brief Returns the position of the mouse cursor.
     * @return Its position, in screen pixels.
     **/
    [[nodiscard]] virtual engine::Vec2f mousePosition() const = 0;
    /**
     * @brief Tells whether the left mouse button was pressed.
     * @return `true` if it was pressed since the last shouldClose().
     **/
    [[nodiscard]] virtual bool isMousePressed() const = 0;
};

/**
 * @struct WindowConfig
 * @brief The settings of the window opened by createGraphics().
 * @param width Its width at startup, in pixels.
 * @param height Its height at startup, in pixels.
 * @param title Its title.
 * @param vsync Whether endFrame() waits for the screen's refresh.
 * @param resizable Whether the user can resize it.
 **/
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
 * @param config The settings of the window.
 * @return The backend, with its window open.
 **/
[[nodiscard]] std::unique_ptr<IGraphics>
createGraphics(const WindowConfig &config);

} // namespace client::gfx

#endif // CLIENT_GRAPHICS_IGRAPHICS_HPP
