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
        render();
    }
}

void Game::update(engine::Duration dt)
{
    backgroundScrollSystem(m_registry, dt);
    const auto now = std::chrono::steady_clock::now();
    const auto payloads = m_network.update(now);
    handleLobbyMessages(payloads);
    if (m_autoJoin && m_network.state() == NetworkClient::State::Connected) {
        applyLobbyChoice(m_autoJoin);
        m_autoJoin.reset();
    }
    const bool inRoom = m_lobby.screen == LobbyState::Screen::InRoom;
    if (inRoom) {
        networkSyncSystem(m_registry, payloads, m_sync, m_sheets);
        smoothingSystem(m_registry, m_sync, dt, m_sheets);
    }
    std::uint8_t actions = 0;
    if (inRoom && m_sync.started) {
        actions = m_bench ? m_bench->input(now)
                          : inputSystem(*m_graphics, m_controls, m_autoFire);
    }
    const std::uint32_t sequence = m_network.sendInput(actions, now);
    if (inRoom) {
        predictionSystem(m_registry, m_sync, sequence, actions, dt);
    }
    if (m_bench && inRoom && m_sync.started) {
        m_bench->record(m_registry, m_sync, m_network, dt, now);
    }
    if (m_sync.gameOver) {
        m_gameOverTime += dt.count();
    }
    shipTiltSystem(m_registry, dt);
    animationSystem(m_registry, dt);

    m_registry.flush();
}

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

void Game::renderWorld()
{
    // The world is in playfield units: scale it to fit the window.
    const engine::Vec2f screen = m_graphics->screenSize();
    const float zoom = std::min(screen.x / rtype::PlayfieldWidth,
                                screen.y / rtype::PlayfieldHeight);
    m_graphics->beginWorld(zoom);
    renderSystem(m_registry, *m_graphics);
    playerLabelSystem(m_registry, m_sync, *m_graphics);
    m_graphics->endWorld();
}

void Game::render()
{
    const MenuKeys keys = m_menuInput.update(*m_graphics);

    m_graphics->beginFrame(gfx::colors::Black);
    backgroundRenderSystem(m_registry, *m_graphics);
    if (m_lobby.screen != LobbyState::Screen::InRoom) {
        if (m_network.state() == NetworkClient::State::Connected) {
            applyLobbyChoice(lobbyScreen(m_lobby, *m_graphics, keys));
        }
    } else {
        renderWorld();
        if (!m_sync.started) {
            const WaitingChoice choice =
                waitingRoomScreen(m_lobby, m_sync, *m_graphics, keys);
            if (choice == WaitingChoice::Start) {
                m_network.startGame(std::chrono::steady_clock::now());
            } else if (choice == WaitingChoice::Leave) {
                leaveRoom();
            }
        } else {
            hudSystem(m_sync, m_autoFire.enabled, *m_graphics);
            const bool keyboard =
                keys.confirm && m_gameOverTime >= GameOverKeyDelay;
            if (m_sync.gameOver &&
                gameOverScreen(m_sync, *m_graphics, keyboard)) {
                leaveRoom();
            }
        }
    }
    drawStatus();
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
