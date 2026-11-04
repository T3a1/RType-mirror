/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SpriteSheets
*/

#ifndef CLIENT_SPRITESHEETS_HPP
#define CLIENT_SPRITESHEETS_HPP

#include "common/components/EntityType.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "graphics/IGraphics.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace client {

/** @brief The sprite sheets of client/assets/sprites/ the game draws. */
enum class Sheet : std::uint8_t {
    Ships,
    Shots,
    EnemyBasic,
    EnemyWave,
    EnemyShooter,
    EnemyMissile,
    EnemySmall,
    EnemyMedium,
    EnemyLarge,
    Explosion,
    Boss,
    Count,
};

/**
 * @brief Loads every sprite sheet once, and unloads them when destroyed.
 *
 * A sheet that cannot be loaded, or a backend without textures (ncurses),
 * gives a texture with id 0: the entities using it are drawn as colored
 * rectangles instead.
 */
class SpriteSheets {
  public:
    /** @param graphics The backend loading the textures; it must outlive
     * the SpriteSheets. */
    explicit SpriteSheets(gfx::IGraphics &graphics);
    ~SpriteSheets();

    SpriteSheets(const SpriteSheets &) = delete;
    SpriteSheets &operator=(const SpriteSheets &) = delete;
    SpriteSheets(SpriteSheets &&) = delete;
    SpriteSheets &operator=(SpriteSheets &&) = delete;

    /** @brief The texture of a sheet, id 0 if it could not be loaded. */
    [[nodiscard]] gfx::Texture texture(Sheet sheet) const;

  private:
    gfx::IGraphics *m_graphics;
    std::array<gfx::Texture, static_cast<std::size_t>(Sheet::Count)>
        m_textures{};
};

/**
 * @brief Gives a networked entity what it needs to be drawn: a Sprite, an
 * Animation if its type is animated, and a ShipTilt for player ships.
 * @param registry The client's registry.
 * @param entity The entity, which must already have its Transform.
 * @param sheets The loaded sprite sheets.
 * @param type The type of the entity.
 * @param slot For a player ship, the slot of its player (1 to 4, the color
 * of the ship), or 0 if unknown.
 **/
void attachSprite(engine::Registry &registry, engine::Entity entity,
                  const SpriteSheets &sheets, rtype::EntityType type,
                  std::uint8_t slot);

/**
 * @brief Creates an explosion where a destroyed ship or enemy was, if it
 * was inside the playfield. Missiles do not explode.
 * @param registry The client's registry.
 * @param entity The destroyed entity, still holding its EntityType and
 * Transform.
 * @param sheets The loaded sprite sheets.
 **/
void spawnExplosion(engine::Registry &registry, engine::Entity entity,
                    const SpriteSheets &sheets);

} // namespace client

#endif // CLIENT_SPRITESHEETS_HPP
