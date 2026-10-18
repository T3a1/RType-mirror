/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** BackgroundScrollSystem
*/

#include "systems/BackgroundScrollSystem.hpp"
#include "components/Background.hpp"
#include "engine/Entity.hpp"

#include <cmath>

namespace client {

void backgroundScrollSystem(engine::Registry &registry, engine::Duration dt)
{
    registry.each<Background>([dt](engine::Entity, Background &background) {
        const auto width = static_cast<float>(background.texture.width);
        if (width <= 0.0F) {
            return;
        }

        background.offset = std::fmod(
            background.offset + (background.speed * dt.count()), width);
        if (background.offset < 0.0F) {
            background.offset += width;
        }
    });
}

} // namespace client
