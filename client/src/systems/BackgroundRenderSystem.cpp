/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** BackgroundRenderSystem
*/

#include "systems/BackgroundRenderSystem.hpp"
#include "components/Background.hpp"
#include "engine/Entity.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace client {

void backgroundRenderSystem(engine::Registry &registry,
                            gfx::IGraphics &graphics)
{
    const engine::Vec2f screen = graphics.screenSize();

    std::vector<const Background *> layers;
    registry.each<Background>(
        [&layers](engine::Entity, const Background &background) {
            layers.push_back(&background);
        });
    std::ranges::stable_sort(layers, {}, &Background::depth);
    for (const Background *background : layers) {
        const auto width = static_cast<float>(background->texture.width);
        const auto height = static_cast<float>(background->texture.height);
        if (width <= 0.0F || height <= 0.0F) {
            continue;
        }
        const float scale = screen.y / height;
        const float tileWidth = width * scale;
        const float start = -background->offset * scale;
        const auto tiles =
            static_cast<int>(std::ceil((screen.x - start) / tileWidth));
        for (int i = 0; i < tiles; ++i) {
            const float x = start + (static_cast<float>(i) * tileWidth);
            graphics.drawTexture(background->texture,
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
                                     .height = screen.y,
                                 },
                                 gfx::colors::White);
        }
    }
}

} // namespace client
