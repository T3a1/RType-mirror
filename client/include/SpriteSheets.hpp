/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** SpriteSheets
*/

#ifndef CLIENT_SPRITESHEETS_HPP
#define CLIENT_SPRITESHEETS_HPP

#include "common/components/EntityType.hpp"
#include "engine/Clock.hpp"
#include "engine/Entity.hpp"
#include "engine/Registry.hpp"
#include "graphics/IGraphics.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace client {

/**
 * @enum Sheet
 * @brief The sprite sheets of client/assets/sprites/ the game draws.
 * @param Ships The ships of the players.
 * @param Shots The shots of the players and the charge effects.
 * @param EnemyBasic The enemy of type Basic.
 * @param EnemyWave The enemy of type Wave.
 * @param EnemyShooter The enemy of type Shooter.
 * @param EnemyMissile The shots of the enemies.
 * @param EnemySmall The enemy of type Small.
 * @param EnemyMedium The enemy of type Medium.
 * @param EnemyLarge The enemy of type Large.
 * @param Explosion The explosions.
 * @param Boss The end-level boss.
 * @param PowerUp The power-ups.
 * @param Hive The second end-level boss.
 * @param Obstacles The walls and destroyable tiles of the levels.
 * @param Rings The spinner and bubble enemies.
 * @param Walker The walker boss and its fireballs.
 * @param Twin The two halves of the twin boss.
 * @param Spores The spore and jelly enemies.
 * @param Crabs The crab enemy.
 * @param Behemoth The behemoth boss: body, head and tail.
 * @param Count The number of sheets, not a sheet.
 **/
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
    PowerUp,
    Hive,
    Obstacles,
    Rings,
    Walker,
    Twin,
    Spores,
    Crabs,
    Behemoth,
    Count,
};

/**
 * @class SpriteSheets
 * @brief Loads every sprite sheet once, and unloads them when destroyed.
 *
 * A sheet that cannot be loaded, or a backend without textures (ncurses),
 * gives a texture with id 0: the entities using it are drawn as colored
 * rectangles instead.
 */
class SpriteSheets {
  public:
    /**
     * @brief Loads the texture of every sheet.
     * @param graphics The backend loading the textures; it must outlive the
     * SpriteSheets.
     **/
    explicit SpriteSheets(gfx::IGraphics &graphics);
    /**
     * @brief Frees the textures.
     **/
    ~SpriteSheets();

    SpriteSheets(const SpriteSheets &) = delete;
    SpriteSheets &operator=(const SpriteSheets &) = delete;
    SpriteSheets(SpriteSheets &&) = delete;
    SpriteSheets &operator=(SpriteSheets &&) = delete;

    /**
     * @brief Returns the texture of a sheet.
     * @param sheet The sheet.
     * @return Its texture, with id 0 if it could not be loaded.
     **/
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
 * @brief Draws a charging glow in front of every player ship whose shot
 * is charging (ChargeState), in world units: call it between beginWorld()
 * and endWorld(), after renderSystem().
 * @param registry The client's registry.
 * @param sheets The loaded sprite sheets.
 * @param graphics The graphics backend to draw with.
 **/
void chargeGlowSystem(engine::Registry &registry, const SpriteSheets &sheets,
                      gfx::IGraphics &graphics);

/**
 * @brief Shows a destroyable tile cracked once it has lost half of its
 * health, and intact otherwise.
 * @param registry The client's registry.
 * @param dt Unused: the frame only depends on the health.
 **/
void tileCrackSystem(engine::Registry &registry, engine::Duration dt);

/**
 * @brief Creates an explosion where a destroyed ship or enemy was, if it
 * was inside the playfield. Missiles do not explode.
 * @param registry The client's registry.
 * @param entity The destroyed entity, still holding its EntityType and
 * Transform.
 * @param sheets The loaded sprite sheets.
 * @return Whether an explosion was created.
 **/
bool spawnExplosion(engine::Registry &registry, engine::Entity entity,
                    const SpriteSheets &sheets);

} // namespace client

#endif // CLIENT_SPRITESHEETS_HPP
