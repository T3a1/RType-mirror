/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SpriteSheets
*/

#include "SpriteSheets.hpp"
#include "common/Playfield.hpp"
#include "common/components/Transform.hpp"
#include "components/Animation.hpp"
#include "components/Sprite.hpp"

#include <span>

namespace client {

namespace {

using gfx::Rect;
using gfx::rgb;

constexpr std::size_t SheetCount = static_cast<std::size_t>(Sheet::Count);

/** @brief The file of each Sheet, relative to the executable. */
constexpr std::array<const char *, SheetCount> SheetPaths{
    "assets/sprites/r-typesheet42.gif", // Ships
    "assets/sprites/r-typesheet1.gif",  // Shots
    "assets/sprites/r-typesheet5.gif",  // EnemyBasic
    "assets/sprites/r-typesheet8.gif",  // EnemyWave
    "assets/sprites/r-typesheet23.gif", // EnemyShooter
    "assets/sprites/r-typesheet43.gif", // EnemyMissile
    "assets/sprites/r-typesheet12.gif", // EnemySmall
    "assets/sprites/r-typesheet27.gif", // EnemyMedium
    "assets/sprites/r-typesheet26.gif", // EnemyLarge
    "assets/sprites/r-typesheet44.gif", // Explosion
};

/**
 * @brief Frames of the same size laid out on one row of a sheet.
 * @param xs The left edge of each frame.
 * @param y The top edge of the row.
 * @param width The width of a frame.
 * @param height The height of a frame.
 * @return The frames, in the order of xs.
 **/
template <std::size_t N>
constexpr std::array<Rect, N> row(const std::array<float, N> &xs, float y,
                                  float width, float height)
{
    std::array<Rect, N> frames{};
    for (std::size_t i = 0; i < N; ++i) {
        frames.at(i) = {
            .x = xs.at(i), .y = y, .width = width, .height = height};
    }
    return frames;
}

/**
 * @brief The left edges of N frames evenly spaced on a row.
 * @param first The left edge of the first frame.
 * @param step The distance between two frames.
 **/
template <std::size_t N>
constexpr std::array<float, N> evenly(float first, float step)
{
    std::array<float, N> xs{};
    for (std::size_t i = 0; i < N; ++i) {
        xs.at(i) = first + (static_cast<float>(i) * step);
    }
    return xs;
}

// The ship sheet has one row per color (cyan, magenta, green, red, blue)
// and five tilts per row, from fully tilted down to fully tilted up.
constexpr std::size_t ShipColors = 5;
constexpr std::size_t ShipTilts = 5;
constexpr float ShipRowStep = 17.0F;

constexpr auto ShipFrames = [] {
    std::array<std::array<Rect, ShipTilts>, ShipColors> frames{};
    for (std::size_t color = 0; color < ShipColors; ++color) {
        frames.at(color) =
            row(evenly<ShipTilts>(1.0F, 33.0F),
                2.0F + (static_cast<float>(color) * ShipRowStep), 32.0F, 15.0F);
    }
    return frames;
}();

constexpr std::array<Rect, 1> PlayerMissileFrames{
    {{.x = 249.0F, .y = 105.0F, .width = 16.0F, .height = 8.0F}}};
// The second half of the sheet plays the same turn backwards: not used.
constexpr auto EnemyBasicFrames =
    row(evenly<8>(5.0F, 33.0F), 6.0F, 21.0F, 24.0F);
constexpr auto EnemyWaveFrames =
    row(evenly<8>(4.0F, 33.0F), 5.0F, 31.0F, 26.0F);
// The second row plays the same turn backwards: not used.
constexpr auto EnemyShooterFrames =
    row(evenly<8>(1.0F, 33.0F), 5.0F, 32.0F, 24.0F);
constexpr auto EnemyMissileFrames =
    row<8>({136.0F, 153.0F, 170.0F, 188.0F, 205.0F, 223.0F, 240.0F, 257.0F},
           6.0F, 7.0F, 6.0F);
constexpr auto EnemySmallFrames =
    row(evenly<6>(1.0F, 17.0F), 1.0F, 17.0F, 17.0F);
constexpr auto EnemyMediumFrames =
    row(evenly<2>(1.0F, 34.0F), 4.0F, 32.0F, 21.0F);
constexpr auto EnemyLargeFrames =
    row<6>({1.0F, 66.0F, 131.0F, 198.0F, 263.0F, 328.0F}, 1.0F, 63.0F, 48.0F);
constexpr auto ExplosionFrames =
    row(evenly<6>(128.0F, 33.0F), 0.0F, 33.0F, 34.0F);

constexpr AnimationClip EnemyBasicClip{.frames = EnemyBasicFrames,
                                       .frameTime = 0.08F};
constexpr AnimationClip EnemyWaveClip{.frames = EnemyWaveFrames,
                                      .frameTime = 0.08F};
constexpr AnimationClip EnemyShooterClip{.frames = EnemyShooterFrames,
                                         .frameTime = 0.08F};
constexpr AnimationClip EnemyMissileClip{.frames = EnemyMissileFrames,
                                         .frameTime = 0.06F};
constexpr AnimationClip EnemySmallClip{.frames = EnemySmallFrames,
                                       .frameTime = 0.1F};
constexpr AnimationClip EnemyMediumClip{.frames = EnemyMediumFrames,
                                        .frameTime = 0.2F};
constexpr AnimationClip EnemyLargeClip{.frames = EnemyLargeFrames,
                                       .frameTime = 0.1F};
constexpr AnimationClip ExplosionClip{
    .frames = ExplosionFrames, .frameTime = 0.07F, .loop = false};

constexpr float ExplosionScale = 2.5F;
constexpr int EffectLayer = 3;

/**
 * @struct Look
 * @brief How one entity type is drawn.
 * @param sheet The sprite sheet holding its frames.
 * @param frames Its frames, all of the same size; the first one is shown
 * if it is not animated.
 * @param scale The drawn size of a sheet pixel, in playfield units.
 * @param fallback Its color when the sheet has no texture.
 * @param layer Its layer.
 * @param clip Its looping animation, or null.
 **/
struct Look {
    Sheet sheet = Sheet::Ships;
    std::span<const Rect> frames;
    float scale = 1.0F;
    gfx::Color fallback = gfx::colors::White;
    int layer = 1;
    const AnimationClip *clip = nullptr;
};

/**
 * @brief Returns how an entity type is drawn.
 * @param type The type of the entity.
 * @param slot For a player ship, its player's slot, or 0 if unknown.
 * @return Its look.
 **/
Look lookOf(rtype::EntityType type, std::uint8_t slot)
{
    switch (type) {
    case rtype::EntityType::PlayerShip: {
        // Slots 1 to 4 get the first four colors, unknown ships the last.
        const std::size_t color =
            slot >= 1 && slot < ShipColors ? slot - 1U : ShipColors - 1;
        return {.sheet = Sheet::Ships,
                .frames = ShipFrames.at(color),
                .scale = 2.5F,
                .fallback = slot != 0 ? rgb(80, 160, 255) : rgb(120, 200, 120),
                .layer = 2};
    }
    case rtype::EntityType::PlayerMissile:
        return {.sheet = Sheet::Shots,
                .frames = PlayerMissileFrames,
                .scale = 3.0F,
                .fallback = rgb(255, 220, 60)};
    case rtype::EntityType::EnemyBasic:
        return {.sheet = Sheet::EnemyBasic,
                .frames = EnemyBasicFrames,
                .scale = 3.0F,
                .fallback = rgb(230, 60, 60),
                .clip = &EnemyBasicClip};
    case rtype::EntityType::EnemyWave:
        return {.sheet = Sheet::EnemyWave,
                .frames = EnemyWaveFrames,
                .scale = 2.5F,
                .fallback = rgb(240, 140, 40),
                .clip = &EnemyWaveClip};
    case rtype::EntityType::EnemyShooter:
        return {.sheet = Sheet::EnemyShooter,
                .frames = EnemyShooterFrames,
                .scale = 2.5F,
                .fallback = rgb(170, 80, 220),
                .clip = &EnemyShooterClip};
    case rtype::EntityType::EnemyMissile:
        return {.sheet = Sheet::EnemyMissile,
                .frames = EnemyMissileFrames,
                .scale = 3.0F,
                .fallback = rgb(255, 100, 180),
                .clip = &EnemyMissileClip};
    case rtype::EntityType::EnemySmall:
        return {.sheet = Sheet::EnemySmall,
                .frames = EnemySmallFrames,
                .scale = 3.0F,
                .fallback = gfx::colors::Red,
                .clip = &EnemySmallClip};
    case rtype::EntityType::EnemyMedium:
        return {.sheet = Sheet::EnemyMedium,
                .frames = EnemyMediumFrames,
                .scale = 3.0F,
                .fallback = rgb(0, 121, 241),
                .clip = &EnemyMediumClip};
    case rtype::EntityType::EnemyLarge:
        return {.sheet = Sheet::EnemyLarge,
                .frames = EnemyLargeFrames,
                .scale = 2.0F,
                .fallback = gfx::colors::Green,
                .clip = &EnemyLargeClip};
    }
    return {.frames = ShipFrames.back()};
}

/**
 * @brief Builds the Sprite showing one frame of a sheet.
 * @param sheets The loaded sprite sheets.
 * @param look The look of the entity.
 * @param frame The frame shown.
 * @return The sprite, a plain rectangle if the sheet has no texture.
 **/
Sprite makeSprite(const SpriteSheets &sheets, const Look &look,
                  const Rect &frame)
{
    const gfx::Texture texture = sheets.texture(look.sheet);
    return {
        .size = {.x = frame.width * look.scale, .y = frame.height * look.scale},
        .tint = texture.id != 0 ? gfx::colors::White : look.fallback,
        .layer = look.layer,
        .texture = texture,
        .source = frame,
    };
}

} // namespace

SpriteSheets::SpriteSheets(gfx::IGraphics &graphics) : m_graphics(&graphics)
{
    for (std::size_t i = 0; i < SheetCount; ++i) {
        m_textures.at(i) = m_graphics->loadTexture(
            SheetPaths.at(i), gfx::TextureFilter::Pixelated);
    }
}

SpriteSheets::~SpriteSheets()
{
    for (const gfx::Texture &texture : m_textures) {
        m_graphics->unloadTexture(texture);
    }
}

gfx::Texture SpriteSheets::texture(Sheet sheet) const
{
    return m_textures.at(static_cast<std::size_t>(sheet));
}

void attachSprite(engine::Registry &registry, engine::Entity entity,
                  const SpriteSheets &sheets, rtype::EntityType type,
                  std::uint8_t slot)
{
    const Look look = lookOf(type, slot);
    if (type == rtype::EntityType::PlayerShip) {
        const Rect &level = look.frames[look.frames.size() / 2];
        registry.emplace<Sprite>(entity, makeSprite(sheets, look, level));
        registry.emplace<ShipTilt>(
            entity,
            ShipTilt{
                .frames = look.frames,
                .previousY = registry.get<rtype::Transform>(entity).position.y,
            });
        return;
    }
    registry.emplace<Sprite>(entity,
                             makeSprite(sheets, look, look.frames.front()));
    if (look.clip != nullptr) {
        registry.emplace<Animation>(entity, Animation{.clip = look.clip});
    }
}

void spawnExplosion(engine::Registry &registry, engine::Entity entity,
                    const SpriteSheets &sheets)
{
    const auto *type = registry.tryGet<rtype::EntityType>(entity);
    const auto *transform = registry.tryGet<rtype::Transform>(entity);
    if (type == nullptr || transform == nullptr ||
        *type == rtype::EntityType::PlayerMissile ||
        *type == rtype::EntityType::EnemyMissile) {
        return;
    }
    const engine::Vec2f position = transform->position;
    if (position.x < 0.0F || position.x > rtype::PlayfieldWidth ||
        position.y < 0.0F || position.y > rtype::PlayfieldHeight) {
        return;
    }
    const Look look{.sheet = Sheet::Explosion,
                    .frames = ExplosionFrames,
                    .scale = ExplosionScale,
                    .fallback = rgb(255, 200, 60),
                    .layer = EffectLayer};
    const engine::Entity explosion = registry.create();
    registry.emplace<rtype::Transform>(explosion,
                                       rtype::Transform{.position = position});
    registry.emplace<Sprite>(explosion,
                             makeSprite(sheets, look, ExplosionFrames.front()));
    registry.emplace<Animation>(explosion, Animation{.clip = &ExplosionClip});
}

} // namespace client
