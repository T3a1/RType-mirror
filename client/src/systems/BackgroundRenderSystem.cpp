/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** BackgroundRenderSystem
*/

#include "systems/BackgroundRenderSystem.hpp"
#include "components/Background.hpp"
#include "engine/Entity.hpp"

#include <cmath>

namespace client {

void backgroundRenderSystem(engine::Registry &registry,
                            gfx::IGraphics &graphics)
{
    const engine::Vec2f screen = graphics.screenSize();
    const float screenWidth = screen.x;
    const float screenHeight = screen.y;

    registry.each<Background>(
        [&graphics, screenWidth, screenHeight](engine::Entity,
                                               const Background &background) {
            const auto width = static_cast<float>(background.texture.width);
            const auto height = static_cast<float>(background.texture.height);
            if (width <= 0.0F || height <= 0.0F) {
                return;
            }
            const float scale = screenHeight / height;
            const float tileWidth = width * scale;
            const float start = -background.offset * scale;
            const auto tiles =
                static_cast<int>(std::ceil((screenWidth - start) / tileWidth));
            for (int i = 0; i < tiles; ++i) {
                const float x = start + (static_cast<float>(i) * tileWidth);
                graphics.drawTexture(background.texture,
                                     gfx::Rect{
                                         .x = 0.0F,
                                         .y = 0.0F,
                                         .width = width,
                                         .height = height,
                                     },
                                     gfx::Rect{
                                         .x = x,
                                         .y = 0.0F,
                                         .width = tileWidth,
                                         .height = screenHeight,
                                     },
                                     gfx::colors::White);
            }
        });
}

} // namespace client
