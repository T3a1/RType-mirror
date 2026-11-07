/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#include "Game.hpp"
#include "common/Playfield.hpp"
#include "components/Background.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/AudioSystem.hpp"
#include "systems/BackgroundRenderSystem.hpp"
#include "systems/BackgroundScrollSystem.hpp"
#include "systems/HudSystem.hpp"
#include "systems/InputSystem.hpp"
#include "systems/RenderSystem.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace client {

namespace {

/**
 * @struct BackgroundLayer
 * @brief One layer of the scrolling background.
 * @param path Its image, relative to the executable.
 * @param speed How fast it scrolls, in image pixels per second.
 * @param depth Its drawing order: the planets under the stars.
 **/
struct BackgroundLayer {
    const char *path;
    float speed;
    int depth;
};

/** @brief The background layers: the stars, nearer, scroll three times
 * faster than the planets behind them, for a feeling of depth. */
constexpr std::array<BackgroundLayer, BackgroundLayerCount> BackgroundLayers{{
    {.path = "assets/backgrounds/planets.png", .speed = 30.0F, .depth = 0},
    {.path = "assets/backgrounds/stars.png", .speed = 90.0F, .depth = 1},
}};

/** @brief How long the keyboard is ignored on the game over screen, in seconds.
 * Space also fires, so a player still firing does not leave it by accident. */
constexpr float GameOverKeyDelay = 1.5F;

/** @brief The left margin of the connection status, in pixels. */
constexpr float statusX = 20.0F;
/** @brief The distance between the status and the bottom, in pixels. */
constexpr float statusBottomMargin = 50.0F;
/** @brief The font size of the status, in pixels. */
constexpr int statusFontSize = 30;

/**
 * @brief Returns what to show when the server refuses the client.
 * @param reason Why it refused.
 * @return The text.
 **/
const char *rejectText(rtype::protocol::RejectReason reason)
{
    switch (reason) {
    case rtype::protocol::RejectReason::ServerFull:
        return "The server is full";
    case rtype::protocol::RejectReason::VersionMismatch:
        return "The server runs another version of the game";
    case rtype::protocol::RejectReason::Banned:
        return "You are banned from this server";
    case rtype::protocol::RejectReason::NotWhitelisted:
        return "This server only accepts listed players";
    }
    return "Connection refused by the server";
}

/**
 * @brief Returns the window and frame rate to use for some settings.
 * @param settings The settings.
 * @return The values for the graphics backend. V-Sync replaces the frame limit.
 **/
gfx::VideoConfig videoConfig(const Settings &settings)
{
    const WindowSize size = WindowSizes.at(settings.windowSize);
    return {.width = size.width,
            .height = size.height,
            .fullscreen = settings.fullscreen,
            .vsync = settings.vsync,
            .frameLimit =
                settings.vsync ? 0 : FrameLimits.at(settings.frameLimit)};
}

/**
 * @brief Returns the window and frame rate to start with. A benchmark ignores
 * the choices of the player, so that its results can be compared.
 * @param options The options of the game.
 * @return The values for the graphics backend.
 **/
gfx::VideoConfig startVideo(const GameOptions &options)
{
    return videoConfig(options.bench ? Settings{} : options.settings);
}

} // namespace

Game::Game(const std::string &host, std::uint16_t port,
           const GameOptions &options)
    : m_network(host, port), m_graphics(gfx::createGraphics({
                                 .width = startVideo(options).width,
                                 .height = startVideo(options).height,
                                 .title = "R-Type",
                                 .vsync = startVideo(options).vsync,
                             })),
      m_audio(audio::createAudio()), m_sounds(*m_audio), m_sheets(*m_graphics),
      m_autoJoin(options.bench && !options.room
                     ? std::optional<std::uint32_t>{rtype::protocol::AnyRoom}
                     : options.room),
      m_controls(options.controls), m_controlsPath(options.controlsPath),
      m_settings(options.settings)
{
    m_sync.interpolate = options.interpolate;
    m_sync.predict = options.predict;
    m_graphics->applyVideo(startVideo(options));
    if (m_autoJoin) {
        m_menu.page = MenuPage::Lobby;
    }
    applyVolumes();
    m_autoFire.enabled = m_settings.autoFire;
    if (options.bench) {
        m_bench.emplace(*options.bench);
        m_audio->setSoundVolume(0.0F);
        m_audio->setMusicVolume(0.0F);
        m_autoFire.enabled = false;
    }
    spawnBackground();
    registerSystems();
    registerRenderSystems();
}

Game::~Game()
{
    for (const gfx::Texture &texture : m_backgroundTextures) {
        m_graphics->unloadTexture(texture);
    }
}

void Game::run()
{
    using Clock = std::chrono::steady_clock;
    constexpr engine::Duration maxFrameTime{0.25};

    Clock::time_point previous = Clock::now();

    while (!m_quitRequested && !m_graphics->shouldClose() &&
           !(m_bench && m_bench->finished())) {
        const Clock::time_point now = Clock::now();
        const engine::Duration dt =
            std::min(engine::Duration{now - previous}, maxFrameTime);
        previous = now;

        update(dt);
        render(dt);
    }
}

/**
 * @brief Registers the logic systems in the scheduler, stage by stage and in
 *the order they run within a stage.
 **/
void Game::registerSystems()
{
    using engine::Duration;
    using engine::Registry;
    using engine::Stage;

    const engine::SystemOptions playing{.when = [this] { return inRoom(); }};

    m_scheduler.add(Stage::Input, "network", [this](Registry &, Duration) {
        m_now = std::chrono::steady_clock::now();
        m_payloads = m_network.update(m_now);
    });
    m_scheduler.add(Stage::Input, "lobby", [this](Registry &, Duration) {
        handleLobbyMessages(m_payloads);
        if (m_autoJoin &&
            m_network.state() == NetworkClient::State::Connected) {
            applyLobbyChoice(m_autoJoin);
            m_autoJoin.reset();
        }
    });
    m_scheduler.add(
        Stage::Input, "networkSync",
        [this](Registry &r, Duration) {
            networkSyncSystem(r, m_payloads, m_sync, m_sheets, m_sounds);
        },
        playing);
    m_scheduler.add(Stage::Input, "sendInput", [this](Registry &, Duration dt) {
        m_actions = 0;
        if (inRoom() && m_sync.started && !m_sync.chat.open) {
            m_actions =
                m_bench ? m_bench->input(m_now)
                        : inputSystem(*m_graphics, m_controls, m_autoFire, dt);
        }
        m_sequence = m_network.sendInput(m_actions, m_now);
    });

    m_scheduler.add(Stage::Simulation, "backgroundScroll",
                    backgroundScrollSystem);
    m_scheduler.add(
        Stage::Simulation, "smoothing",
        [this](Registry &r, Duration dt) {
            smoothingSystem(r, m_sync, dt, m_sheets, m_sounds);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "prediction",
        [this](Registry &r, Duration dt) {
            predictionSystem(r, m_sync, m_sequence, m_actions, dt);
        },
        playing);
    m_scheduler.add(
        Stage::Simulation, "bench",
        [this](Registry &r, Duration dt) {
            m_bench->record(r, m_sync, m_network, dt, m_now);
        },
        {.when = [this] { return m_bench && inRoom() && m_sync.started; }});
    m_scheduler.add(
        Stage::Simulation, "gameOverTimer",
        [this](Registry &, Duration dt) { m_gameOverTime += dt.count(); },
        {.when = [this] { return m_sync.gameOver; }});
    m_scheduler.add(Stage::Simulation, "levelBanner",
                    [this](Registry &, Duration dt) {
                        m_sync.levelBanner =
                            std::max(m_sync.levelBanner - dt.count(), 0.0F);
                    });
    m_scheduler.add(Stage::Simulation, "playerNotices",
                    [this](Registry &, Duration dt) {
                        updatePlayerNotices(m_sync.notices, dt.count());
                    });
    m_scheduler.add(Stage::Simulation, "chatLines",
                    [this](Registry &, Duration dt) {
                        updateChatLines(m_sync.chat, dt.count());
                    });
    m_scheduler.add(Stage::Simulation, "shipTilt", shipTiltSystem);
    m_scheduler.add(Stage::Simulation, "animation", animationSystem);
    m_scheduler.add(Stage::Simulation, "tileCrack", tileCrackSystem);

    m_scheduler.add(Stage::Output, "audio", [this](Registry &, Duration) {
        audioSystem(m_registry, inRoom() && m_sync.started && !m_sync.gameOver,
                    m_sounds);
    });
}

/**
 * @brief Adds a drawing system to a layer. Within a layer, systems draw in the
 * order they were added.
 * @param layer The layer the system draws in.
 * @param name The name of the system, for logs and debugging.
 * @param system The system to run.
 * @param options When the system runs.
 **/
void Game::addRender(Layer layer, std::string name, engine::System system,
                     engine::SystemOptions options)
{
    m_layers.at(static_cast<std::size_t>(layer))
        .add(engine::Stage::Output, std::move(name), std::move(system),
             std::move(options));
}

/**
 * @brief Registers the drawing systems in their layer, from back to front.
 **/
void Game::registerRenderSystems()
{
    using engine::Duration;
    using engine::Registry;

    addRender(Layer::Background, "background", [this](Registry &r, Duration) {
        backgroundRenderSystem(r, *m_graphics);
    });
    addRender(Layer::World, "sprites",
              [this](Registry &r, Duration) { renderSystem(r, *m_graphics); });
    addRender(Layer::World, "chargeGlow", [this](Registry &r, Duration) {
        chargeGlowSystem(r, m_sheets, *m_graphics);
    });
    addRender(Layer::World, "playerLabels",
              [this](Registry &r, Duration) {
                  playerLabelSystem(r, m_sync, *m_graphics);
              },
              {.when = [this] { return m_settings.playerLabels; }});
    addRender(Layer::World, "powerUpLabels", [this](Registry &r, Duration) {
        powerUpLabelSystem(r, *m_graphics);
    });
    addRender(Layer::Overlay, "screens",
              [this](Registry &, Duration) { drawScreens(); });
    addRender(Layer::Overlay, "status",
              [this](Registry &, Duration) { drawStatus(); });
}

bool Game::inRoom() const
{
    return m_lobby.screen == LobbyState::Screen::InRoom;
}

void Game::update(engine::Duration dt) { m_scheduler.run(m_registry, dt); }

void Game::handleLobbyMessages(const std::vector<rtype::protocol::Payload> &all)
{
    for (const auto &payload : all) {
        if (const auto *list =
                std::get_if<rtype::protocol::RoomList>(&payload)) {
            m_lobby.rooms = list->rooms;
        } else if (const auto *joined =
                       std::get_if<rtype::protocol::RoomJoined>(&payload)) {
            m_lobby.screen = LobbyState::Screen::InRoom;
            m_lobby.room = joined->room;
            m_lobby.levels = joined->levels;
            m_lobby.settings.level =
                joined->levels > 0 ? 1 : rtype::protocol::EndlessLevel;
            if (m_bench) {
                m_network.startGame(m_lobby.settings,
                                    std::chrono::steady_clock::now());
            }
        } else if (std::holds_alternative<rtype::protocol::RoomRefused>(
                       payload)) {
            m_lobby.screen = LobbyState::Screen::Lobby;
            m_lobby.refused = true;
            m_sounds.play(audio::SoundEvent::MenuRefused);
        } else if (std::holds_alternative<rtype::protocol::RoomClosed>(
                       payload)) {
            leaveRoom();
            m_lobby.closed = true;
        }
    }
}

void Game::applyLobbyChoice(std::optional<std::uint32_t> room)
{
    if (room) {
        m_lobby.screen = LobbyState::Screen::Joining;
        m_lobby.refused = false;
        m_lobby.closed = false;
        m_network.joinRoom(*room, std::chrono::steady_clock::now());
    }
}

void Game::leaveRoom()
{
    m_network.leaveRoom(std::chrono::steady_clock::now());
    const bool interpolate = m_sync.interpolate;
    const bool predict = m_sync.predict;
    m_sync = SyncState{};
    m_sync.interpolate = interpolate;
    m_sync.predict = predict;
    m_registry = engine::Registry{};
    createBackground();
    m_lobby.screen = LobbyState::Screen::Lobby;
    m_lobby.lobbyCursor = MenuCursor{};
    m_lobby.waitingCursor = MenuCursor{};
    m_gameOverTime = 0.0F;
}

/**
 * @brief Handles the back and leave keys: in a room, the leave key leaves it;
 * out of a room, the back key stops waiting for a new key or goes back to the
 * main menu. They do nothing in the main menu or while a room is being joined,
 * and quit during a benchmark.
 **/
void Game::handleBackKey()
{
    const bool back = m_menuKeys.back;
    if (!back && !m_menuKeys.leave) {
        return;
    }
    if (m_bench) {
        m_quitRequested = true;
    } else if (inRoom()) {
        if (m_menuKeys.leave) {
            m_sounds.play(audio::SoundEvent::MenuBack);
            leaveRoom();
        }
    } else if (back && m_menu.settings.capture.slot()) {
        m_sounds.play(audio::SoundEvent::MenuBack);
        m_menu.settings.capture.cancel();
    } else if (back && m_menu.page != MenuPage::Main &&
               m_lobby.screen != LobbyState::Screen::Joining) {
        m_sounds.play(audio::SoundEvent::MenuBack);
        m_menu.page = MenuPage::Main;
    }
}

/**
 * @brief Returns what the menus need for this frame.
 * @return The graphics backend, the sounds and the menu keys.
 **/
MenuContext Game::menuContext()
{
    return {
        .graphics = m_graphics.get(), .sounds = &m_sounds, .keys = m_menuKeys};
}

/**
 * @brief Applies the choice of the main menu.
 * @param choice What the player chose.
 **/
void Game::applyMainChoice(MainChoice choice)
{
    switch (choice) {
    case MainChoice::None:
        break;
    case MainChoice::Play:
        m_menu.page = MenuPage::Lobby;
        break;
    case MainChoice::Settings:
        m_menu.page = MenuPage::Settings;
        break;
    case MainChoice::Accessibility:
        m_menu.page = MenuPage::Accessibility;
        break;
    case MainChoice::HowToPlay:
        m_menu.page = MenuPage::HowToPlay;
        break;
    case MainChoice::Credits:
        m_menu.page = MenuPage::Credits;
        break;
    case MainChoice::Quit:
        m_quitRequested = true;
        break;
    }
}

/**
 * @brief Gives the volumes of the settings to the audio backend.
 **/
void Game::applyVolumes()
{
    m_audio->setMusicVolume(m_settings.musicVolume);
    m_audio->setSoundVolume(m_settings.soundVolume);
}

/**
 * @brief Ends a frame of a page of settings: saves what changed, and goes back
 * to the main menu if the player chose to.
 * @param result What the player did on the page.
 **/
void Game::finishSettingsPage(const SettingsResult &result)
{
    if (result.changed) {
        saveSettingsFile(SettingsPath, m_settings);
    }
    if (result.controlsChanged) {
        saveControlsFile(m_controlsPath, m_controls);
    }
    if (result.back) {
        m_menu.page = MenuPage::Main;
    }
}

/**
 * @brief Draws the Settings page, and applies and saves what changed.
 **/
void Game::drawSettings()
{
    const SettingsResult result = settingsPage(menuContext(), m_menu.settings,
                                               m_settings, m_controls.keyboard);
    if (result.changed) {
        applyVolumes();
    }
    if (result.videoChanged) {
        m_graphics->applyVideo(videoConfig(m_settings));
    }
    finishSettingsPage(result);
}

/**
 * @brief Draws the Accessibility page, and applies and saves what changed.
 * Auto-fire only follows its own checkbox, so that changing another option
 * keeps what the player chose with its key.
 **/
void Game::drawAccessibility()
{
    const bool autoFire = m_settings.autoFire;
    const SettingsResult result = accessibilityPage(
        menuContext(), m_menu.accessibilityCursor, m_settings);
    if (m_settings.autoFire != autoFire) {
        m_autoFire.enabled = m_settings.autoFire;
    }
    finishSettingsPage(result);
}

/**
 * @brief Draws the page of the menus the player is on.
 **/
void Game::drawMenu()
{
    const MenuContext context = menuContext();
    switch (m_menu.page) {
    case MenuPage::Main:
        applyMainChoice(
            mainMenuPage(context, m_menu.mainCursor,
                         m_network.state() == NetworkClient::State::Connected));
        break;
    case MenuPage::Lobby:
        drawLobby();
        break;
    case MenuPage::Settings:
        drawSettings();
        break;
    case MenuPage::Accessibility:
        drawAccessibility();
        break;
    case MenuPage::HowToPlay:
        if (howToPlayPage(context, m_menu.howToPlayCursor,
                          m_controls.keyboard)) {
            m_menu.page = MenuPage::Main;
        }
        break;
    case MenuPage::Credits:
        if (creditsPage(context, m_menu.creditsCursor)) {
            m_menu.page = MenuPage::Main;
        }
        break;
    }
}

/**
 * @brief Draws the menu of the rooms once connected, and joins the chosen room.
 **/
void Game::drawLobby()
{
    if (m_network.state() != NetworkClient::State::Connected) {
        return;
    }
    applyLobbyChoice(lobbyScreen(m_lobby, menuContext()));
}

/**
 * @brief Draws the waiting room, and starts the game or leaves the room.
 **/
void Game::drawWaitingRoom()
{
    const WaitingChoice choice =
        waitingRoomScreen(m_lobby, m_sync, menuContext());
    if (choice == WaitingChoice::Start) {
        m_network.startGame(m_lobby.settings, std::chrono::steady_clock::now());
    } else if (choice == WaitingChoice::Leave) {
        leaveRoom();
    }
}

/**
 * @brief Draws the HUD of a running game, then its banners or game over screen.
 **/
void Game::drawGame()
{
    hudSystem(m_sync, m_autoFire.enabled, *m_graphics);
    if (!m_sync.gameOver) {
        bossHealthBar(m_registry, *m_graphics);
        levelBannerScreen(m_sync, *m_graphics);
    }
    // The fire key also confirms: wait before listening to it on this screen.
    MenuContext context = menuContext();
    context.keys.confirm =
        context.keys.confirm && m_gameOverTime >= GameOverKeyDelay;
    if (m_sync.gameOver && gameOverScreen(m_sync, context)) {
        leaveRoom();
    }
}

void Game::drawScreens()
{
    handleBackKey();
    if (!inRoom()) {
        drawMenu();
    } else if (!m_sync.started) {
        drawWaitingRoom();
    } else {
        drawGame();
    }
    playerNoticeSystem(m_sync, *m_graphics);
    drawChat();
}

/**
 * @brief Gives the keyboard to the chat while it is open: the characters go to
 * the draft, Enter sends it, Escape cancels it, and the menus and the ship
 * receive nothing. Otherwise, the chat key of a room opens it, unless the
 * backend cannot read text or a benchmark is running.
 **/
void Game::updateChat()
{
    ChatState &chat = m_sync.chat;
    const std::u32string typed = m_graphics->takeTextInput();
    const bool chatKey = isHeld(*m_graphics, m_controls, Control::Chat);
    const bool chatPressed = chatKey && !m_chatKeyHeld;
    m_chatKeyHeld = chatKey;
    const bool enter = m_graphics->isKeyDown(gfx::Key::Enter);
    const bool enterPressed = enter && !m_enterHeld;
    m_enterHeld = enter;

    if (!chat.open) {
        if (!chatPressed || !inRoom() || m_bench ||
            !m_graphics->hasTextInput()) {
            return;
        }
        chat.open = true;
    } else if (m_menuKeys.back) {
        std::ignore = closeChat(chat);
    } else if (enterPressed) {
        if (const auto line = closeChat(chat)) {
            m_network.sendChat(*line, std::chrono::steady_clock::now());
        }
    } else {
        typeInDraft(chat.draft, typed);
    }
    m_menuKeys = MenuKeys{};
}

/**
 * @brief Draws the chat of the room, and in the waiting room the key that opens
 * it.
 **/
void Game::drawChat()
{
    if (!inRoom()) {
        return;
    }
    const std::vector<gfx::Key> &keys =
        m_controls.keyboard.at(static_cast<std::size_t>(Control::Chat));
    if (!m_sync.started && !m_sync.chat.open && m_sync.chat.lines.empty() &&
        !keys.empty() && m_graphics->hasTextInput()) {
        chatHint(keyName(keys.front()), *m_graphics);
    }
    chatOverlay(m_sync.chat, *m_graphics);
}

void Game::render(engine::Duration dt)
{
    const auto draw = [&](Layer layer) {
        m_layers.at(static_cast<std::size_t>(layer)).run(m_registry, dt);
    };

    m_menuKeys = m_menuInput.update(*m_graphics);
    updateChat();

    m_graphics->beginFrame(gfx::colors::Black);
    draw(Layer::Background);
    if (inRoom()) {
        // The world is in playfield units: scale it to fit the window.
        const engine::Vec2f screen = m_graphics->screenSize();
        m_graphics->beginWorld(std::min(screen.x / rtype::PlayfieldWidth,
                                        screen.y / rtype::PlayfieldHeight));
        draw(Layer::World);
        m_graphics->endWorld();
    }
    draw(Layer::Overlay);
    m_graphics->endFrame();
}

void Game::spawnBackground()
{
    for (std::size_t i = 0; i < BackgroundLayers.size(); ++i) {
        m_backgroundTextures.at(i) = m_graphics->loadTexture(
            BackgroundLayers.at(i).path, gfx::TextureFilter::Pixelated);
    }
    createBackground();
}

void Game::createBackground()
{
    for (std::size_t i = 0; i < BackgroundLayers.size(); ++i) {
        const engine::Entity background = m_registry.create();
        m_registry.emplace<Background>(
            background, Background{.texture = m_backgroundTextures.at(i),
                                   .speed = BackgroundLayers.at(i).speed,
                                   .depth = BackgroundLayers.at(i).depth});
    }
}

void Game::drawStatus() const
{
    const char *text = nullptr;
    switch (m_network.state()) {
    case NetworkClient::State::Connecting:
        text = "Connecting...";
        break;
    case NetworkClient::State::Connected:
        return;
    case NetworkClient::State::Rejected:
        text = rejectText(m_network.rejectReason());
        break;
    case NetworkClient::State::Lost:
        text = "Connection to the server lost";
        break;
    case NetworkClient::State::Closed:
        text = m_network.disconnectReason() ==
                       rtype::protocol::DisconnectReason::Kicked
                   ? "Kicked from the server"
                   : "Disconnected by the server";
        break;
    }
    m_graphics->drawText(
        text,
        {.x = statusX, .y = m_graphics->screenSize().y - statusBottomMargin},
        statusFontSize, gfx::colors::OffWhite);
}

} // namespace client
