/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#ifndef CLIENT_GAME_HPP
#define CLIENT_GAME_HPP

#include "Bench.hpp"
#include "Controls.hpp"
#include "MenuPages.hpp"
#include "Settings.hpp"
#include "SpriteSheets.hpp"
#include "audio/IAudio.hpp"
#include "audio/SoundBoard.hpp"
#include "engine/Clock.hpp"
#include "engine/Registry.hpp"
#include "engine/Scheduler.hpp"
#include "graphics/IGraphics.hpp"
#include "network/NetworkClient.hpp"
#include "systems/InputSystem.hpp"
#include "systems/LobbySystem.hpp"
#include "systems/NetworkSyncSystem.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace client {

/**
 * @struct GameOptions
 * @brief The command-line options of the client.
 * @param interpolate Whether the other entities are interpolated.
 * @param predict Whether the local ship is predicted.
 * @param room The room to enter directly, instead of showing the lobby. A
 * benchmark without a room enters the first one with a free slot and starts
 * its game at once.
 * @param bench The network benchmark to run instead of the keyboard, if
 * any.
 * @param controls The keys bound to each control.
 * @param settings The settings of the player.
 **/
struct GameOptions {
    bool interpolate = true;
    bool predict = true;
    std::optional<std::uint32_t> room;
    std::optional<BenchConfig> bench;
    Controls controls = defaultControls();
    Settings settings;
};

/**
 * @enum Layer
 * @brief A drawing layer of a frame. Layers are drawn in this order.
 * @param Background Screen coordinates, under the world.
 * @param World Playfield coordinates, scaled to fit the window.
 * @param Overlay Screen coordinates, over the world (HUD, menus, status).
 * @param Count Number of layers, not a layer.
 **/
enum class Layer : std::uint8_t {
    Background,
    World,
    Overlay,
    Count,
};

/**
 * @class Game
 * @brief The client game: its own registry (a mirror of the server's world plus
 * local-only entities such as the background), and a loop that runs once per
 * displayed frame with a variable time step.
 **/
class Game {
  public:
    /**
     * @brief Opens the window and starts connecting to the server.
     * @param host The server's host name or IP address.
     * @param port The server's UDP port.
     * @param options The options of the command line.
     * @throws engine::net::NetworkError If the address cannot be resolved.
     * @throws std::runtime_error If the window cannot be opened.
     **/
    Game(const std::string &host, std::uint16_t port,
         const GameOptions &options = {});
    /**
     * @brief Tells the server the client leaves and closes the window.
     **/
    ~Game();

    Game(const Game &) = delete;
    Game &operator=(const Game &) = delete;
    Game(Game &&) = delete;
    Game &operator=(Game &&) = delete;

    /**
     * @brief Runs the game until the window is closed or the benchmark is over.
     **/
    void run();

  private:
    void update(engine::Duration dt);
    void render(engine::Duration dt);
    void registerSystems();
    void registerRenderSystems();
    void addRender(Layer layer, std::string name, engine::System system);
    [[nodiscard]] bool inRoom() const;
    void handleBackKey();
    [[nodiscard]] MenuContext menuContext();
    void applyMainChoice(MainChoice choice);
    void applyVolumes();
    void drawMenu();
    void drawSettings();
    void drawLobby();
    void drawWaitingRoom();
    void drawGame();
    void drawScreens();
    void handleLobbyMessages(const std::vector<rtype::protocol::Payload> &all);
    void applyLobbyChoice(std::optional<std::uint32_t> room);
    void leaveRoom();
    void createBackground();
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
    std::unique_ptr<audio::IAudio> m_audio;
    audio::SoundBoard m_sounds;
    SpriteSheets m_sheets;
    engine::Registry m_registry;
    gfx::Texture m_backgroundTexture;
    SyncState m_sync;
    bool m_quitRequested = false;
    LobbyState m_lobby;
    MenuState m_menu;
    MenuInput m_menuInput;
    std::optional<std::uint32_t> m_autoJoin;
    float m_gameOverTime = 0.0F;
    std::optional<Bench> m_bench;
    Controls m_controls;
    Settings m_settings;
    AutoFire m_autoFire;
    NetworkClient::TimePoint m_now;
    std::vector<rtype::protocol::Payload> m_payloads;
    std::uint8_t m_actions = 0;
    std::uint32_t m_sequence = 0;
    MenuKeys m_menuKeys;
    engine::Scheduler m_scheduler;
    std::array<engine::Scheduler, static_cast<std::size_t>(Layer::Count)>
        m_layers;
};

} // namespace client

#endif // CLIENT_GAME_HPP
