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
        const engine::Vec2f topLeft =
            command.transform->position - command.sprite->size * 0.5;
        graphics.drawRectangle(gfx::Rect{.x = topLeft.x,
                                         .y = topLeft.y,
                                         .width = command.sprite->size.x,
                                         .height = command.sprite->size.y},
                               command.sprite->tint);
    }
}

} // namespace client
