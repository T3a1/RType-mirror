/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** RaylibGraphics
*/

#ifndef CLIENT_GRAPHICS_RAYLIBGRAPHICS_HPP
#define CLIENT_GRAPHICS_RAYLIBGRAPHICS_HPP

#include "graphics/IGraphics.hpp"

#include <raylib.h>

#include <cstdint>
#include <unordered_map>

namespace client::gfx {

/** @brief IGraphics backend using Raylib. Only one may exist at a time. */
class RaylibGraphics final : public IGraphics {
  public:
    // Throws if the window cannot be opened.
    explicit RaylibGraphics(const WindowConfig &config);
    ~RaylibGraphics() override;

    RaylibGraphics(const RaylibGraphics &) = delete;
    RaylibGraphics &operator=(const RaylibGraphics &) = delete;
    RaylibGraphics(RaylibGraphics &&) = delete;
    RaylibGraphics &operator=(RaylibGraphics &&) = delete;

    [[nodiscard]] bool shouldClose() override;
    [[nodiscard]] engine::Vec2f screenSize() const override;

    void beginFrame(Color clear) override;
    void endFrame() override;
    void beginWorld(float zoom) override;
    void endWorld() override;

    [[nodiscard]] Texture loadTexture(const std::string &path) override;
    void unloadTexture(Texture texture) override;

    void drawRectangle(Rect rect, Color color) override;
    void drawTexture(Texture texture, Rect source, Rect dest,
                     Color tint) override;
    void drawText(const std::string &text, engine::Vec2f position, int fontSize,
                  Color color) override;

    [[nodiscard]] bool isKeyDown(Key key) const override;

  private:
    std::unordered_map<std::uint32_t, Texture2D> m_textures;
    std::uint32_t m_nextTexture = 1;
};

} // namespace client::gfx

#endif // CLIENT_GRAPHICS_RAYLIBGRAPHICS_HPP
