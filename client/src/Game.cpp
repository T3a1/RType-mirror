/*
** EPITECH PROJECT, 2026
** r-type_client
** File description:
** Game
*/

#include "Game.hpp"
#include "Window.hpp"
#include "common/Playfield.hpp"
#include "components/Background.hpp"
#include "systems/AnimationSystem.hpp"
#include "systems/BackgroundRenderSystem.hpp"
#include "systems/BackgroundScrollSystem.hpp"
#include "systems/HudSystem.hpp"
#include "systems/InputSystem.hpp"
#include "systems/RenderSystem.hpp"

#include <algorithm>
#include <chrono>
#include <string>
#include <utility>

namespace client {

namespace {

constexpr const char *backgroundPath = "assets/bg.jpg";
constexpr float backgroundSpeed = 120.0F;

// Space also fires: it only leaves the game over screen after this delay, so
// that a player still firing does not leave it by accident.
constexpr float GameOverKeyDelay = 1.5F;

constexpr float statusX = 20.0F;
constexpr float statusBottomMargin = 50.0F;
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

} // namespace

Game::Game(const std::string &host, std::uint16_t port,
           const GameOptions &options)
    : m_network(host, port), m_graphics(gfx::createGraphics({
                                 .width = windowWidth,
                                 .height = windowHeight,
                                 .title = "R-Type",
                             })),
      m_sheets(*m_graphics),
      m_autoJoin(options.bench && !options.room
                     ? std::optional<std::uint32_t>{rtype::protocol::AnyRoom}
                     : options.room),
      m_controls(options.controls)
{
    m_sync.interpolate = options.interpolate;
    m_sync.predict = options.predict;
    if (options.bench) {
        m_bench.emplace(*options.bench);
    }
    spawnBackground();
    registerSystems();
    registerRenderSystems();
}

Game::~Game() { m_graphics->unloadTexture(m_backgroundTexture); }

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
 * Registers the logic systems in the scheduler, stage by stage and in the
 * order they run within a stage.
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
            networkSyncSystem(r, m_payloads, m_sync, m_sheets);
        },
        playing);
    m_scheduler.add(Stage::Input, "sendInput", [this](Registry &, Duration dt) {
        m_actions = 0;
        if (inRoom() && m_sync.started) {
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
            smoothingSystem(r, m_sync, dt, m_sheets);
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
    m_scheduler.add(Stage::Simulation, "shipTilt", shipTiltSystem);
    m_scheduler.add(Stage::Simulation, "animation", animationSystem);
}

/**
 * Adds a drawing system to a layer. Within a layer, systems draw in the
 * order they were added.
 * @param layer The layer the system draws in.
 * @param name The name of the system, for logs and debugging.
 * @param system The system to run.
 **/
void Game::addRender(Layer layer, std::string name, engine::System system)
{
    m_layers.at(static_cast<std::size_t>(layer))
        .add(engine::Stage::Output, std::move(name), std::move(system));
}

/**
 * Registers the drawing systems in their layer, from back to front.
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
    addRender(Layer::World, "playerLabels", [this](Registry &r, Duration) {
        playerLabelSystem(r, m_sync, *m_graphics);
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
            if (m_bench) {
                m_network.startGame(std::chrono::steady_clock::now());
            }
        } else if (std::holds_alternative<rtype::protocol::RoomRefused>(
                       payload)) {
            m_lobby.screen = LobbyState::Screen::Lobby;
            m_lobby.refused = true;
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
    m_lobby.selected = 0;
    m_gameOverTime = 0.0F;
}

void Game::drawScreens()
{
    if (!inRoom()) {
        if (m_network.state() == NetworkClient::State::Connected) {
            applyLobbyChoice(lobbyScreen(m_lobby, *m_graphics, m_menuKeys));
        }
    } else if (!m_sync.started) {
        const WaitingChoice choice =
            waitingRoomScreen(m_lobby, m_sync, *m_graphics, m_menuKeys);
        if (choice == WaitingChoice::Start) {
            m_network.startGame(std::chrono::steady_clock::now());
        } else if (choice == WaitingChoice::Leave) {
            leaveRoom();
        }
    } else {
        hudSystem(m_sync, m_autoFire.enabled, *m_graphics);
        if (!m_sync.gameOver) {
            bossHealthBar(m_registry, *m_graphics);
            levelBannerScreen(m_sync, *m_graphics);
        }
        const bool keyboard =
            m_menuKeys.confirm && m_gameOverTime >= GameOverKeyDelay;
        if (m_sync.gameOver && gameOverScreen(m_sync, *m_graphics, keyboard)) {
            leaveRoom();
        }
    }
    playerNoticeSystem(m_sync, *m_graphics);
}

void Game::render(engine::Duration dt)
{
    const auto draw = [&](Layer layer) {
        m_layers.at(static_cast<std::size_t>(layer)).run(m_registry, dt);
    };

    m_menuKeys = m_menuInput.update(*m_graphics);

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
    m_backgroundTexture =
        m_graphics->loadTexture(backgroundPath, gfx::TextureFilter::Smooth);
    createBackground();
}

void Game::createBackground()
{
    const engine::Entity background = m_registry.create();
    m_registry.emplace<Background>(
        background,
        Background{.texture = m_backgroundTexture, .speed = backgroundSpeed});
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
