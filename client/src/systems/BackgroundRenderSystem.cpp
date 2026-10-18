/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** BackgroundRenderSystem
*/

#include "systems/BackgroundRenderSystem.hpp"
#include "components/Background.hpp"
#include "engine/Entity.hpp"

#include <raylib.h>

#include <cmath>

namespace client {

void backgroundRenderSystem(engine::Registry &registry)
{
    const auto screenWidth = static_cast<float>(GetScreenWidth());
    const auto screenHeight = static_cast<float>(GetScreenHeight());

    registry.each<Background>(
        [screenWidth, screenHeight](engine::Entity,
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
                DrawTexturePro(background.texture,
                               Rectangle{
                                   .x = 0.0F,
                                   .y = 0.0F,
                                   .width = width,
                                   .height = height,
                               },
                               Rectangle{
                                   .x = x,
                                   .y = 0.0F,
                                   .width = tileWidth,
                                   .height = screenHeight,
                               },
                               Vector2{.x = 0.0F, .y = 0.0F}, 0.0F, WHITE);
            }
        });
}

} // namespace client
