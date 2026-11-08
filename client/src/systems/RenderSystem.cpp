/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** RenderSystem
*/

#include "systems/RenderSystem.hpp"
#include "common/components/Transform.hpp"
#include "components/Sprite.hpp"
#include "engine/Entity.hpp"

#include <algorithm>
#include <vector>

namespace client {

namespace {

/**
 * @struct DrawCommand
 * @brief An entity to draw, kept to sort the entities by layer.
 * @param transform Its position.
 * @param sprite Its sprite.
 **/
struct DrawCommand {
    const rtype::Transform *transform;
    const Sprite *sprite;
};

} // namespace

void renderSystem(engine::Registry &registry, gfx::IGraphics &graphics)
{
    std::vector<DrawCommand> commands;
    registry.each<rtype::Transform, Sprite>(
        [&commands](engine::Entity, const rtype::Transform &transform,
                    const Sprite &sprite) {
            commands.push_back({.transform = &transform, .sprite = &sprite});
        });

    std::ranges::stable_sort(commands, {}, [](const DrawCommand &command) {
        return command.sprite->layer;
    });

    for (const DrawCommand &command : commands) {
        const Sprite &sprite = *command.sprite;
        const engine::Vec2f topLeft =
            command.transform->position - sprite.size * 0.5;
        const gfx::Rect dest{.x = topLeft.x,
                             .y = topLeft.y,
                             .width = sprite.size.x,
                             .height = sprite.size.y};
        if (sprite.texture.id != 0) {
            graphics.drawTexture(sprite.texture, sprite.source, dest,
                                 sprite.tint);
        } else {
            graphics.drawRectangle(dest, sprite.tint);
        }
    }
}

} // namespace client
