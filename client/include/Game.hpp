/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#ifndef CLIENT_GAME_HPP
#define CLIENT_GAME_HPP

#include "engine/Clock.hpp"
#include "engine/Registry.hpp"
#include "engine/Scheduler.hpp"
#include "graphics/IGraphics.hpp"
#include "network/NetworkClient.hpp"
#include "systems/NetworkSyncSystem.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace client {

/**
 * @enum Layer
 * @brief A drawing layer of a frame. Layers are drawn in this order.
 **/
enum class Layer : std::uint8_t {
    Background, ///< Screen coordinates, under the world.
    World,      ///< Playfield coordinates, scaled to fit the window.
    Overlay,    ///< Screen coordinates, over the world (HUD, status).
    Count,      ///< Number of layers, not a layer.
};

// The client game: its own registry (a mirror of the server's world plus
// local-only entities such as the background), and a loop that runs once
// per displayed frame with a variable time step. The logic systems run in a
// scheduler, the drawing systems in one scheduler per layer.
class Game {
  public:
    // Throws if the server address cannot be resolved or the window cannot
    // be opened.
    Game(const std::string &host, std::uint16_t port);
    ~Game();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;
    Game(Game &&) = delete;
    Game &operator=(Game &&) = delete;

    // Runs until the window is closed.
    void run();

  private:
    void update(engine::Duration dt);
    void render(engine::Duration dt);
    void registerSystems();
    void registerRenderSystems();
    void addRender(Layer layer, std::string name, engine::System system);
    /**
     * @brief Loads the background texture and creates the entity scrolling it.
     *
     * The texture is loaded from the assets directory next to the executable
     * and stays owned by the Game, which unloads it in its destructor. If it
     * cannot be loaded, the window simply has no background.
     */
    void spawnBackground();
    void drawStatus() const;
    NetworkClient m_network;
    std::unique_ptr<gfx::IGraphics> m_graphics;
    engine::Registry m_registry;
    gfx::Texture m_backgroundTexture;
    SyncState m_sync;
    engine::Scheduler m_scheduler;
    std::array<engine::Scheduler, static_cast<std::size_t>(Layer::Count)>
        m_layers;
};

} // namespace client

#endif // CLIENT_GAME_HPP
