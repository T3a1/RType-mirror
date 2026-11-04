/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** NetworkServer
*/

#ifndef SERVER_NETWORKSERVER_HPP
#define SERVER_NETWORKSERVER_HPP
#include "AccessList.hpp"
#include "common/protocol/ByteWriter.hpp"
#include "common/protocol/Connection.hpp"
#include "common/protocol/Packet.hpp"
#include "common/protocol/ReliableChannel.hpp"
#include "engine/CommandConsole.hpp"
#include "engine/MessageQueue.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/network/IUdpSocket.hpp"
#include "network/NetworkMessages.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace server {

using TimePoint = rtype::protocol::ReliableChannel::TimePoint;

/**
 * @struct Session
 * @brief The network thread's state for one connected client.
 * @param id The ClientId shared with the game thread.
 * @param room The room of the client, AnyInstance in the lobby.
 * @param link The queues of that room, null in the lobby.
 * @param channel The reliability state of the connection.
 * @param lastHeard When the last valid packet was received, for the timeout.
 * @param kicked Set when the client must be disconnected at the next
 * update.
 **/
struct Session {
    std::uint32_t id{};
    engine::InstanceId room = engine::AnyInstance;
    std::shared_ptr<Rooms::Link> link;
    rtype::protocol::ReliableChannel channel;
    TimePoint lastHeard;
    bool kicked = false;
};

using SessionIterator = std::map<engine::net::Endpoint, Session>::iterator;

/**
 * @struct AdminRequest
 * @brief A line of the administration console for the network thread.
 * @param line The command.
 * @param answer Receives its answer.
 **/
struct AdminRequest {
    std::string line;
    std::promise<std::string> answer;
};

/**
 * @brief Queue from the administration console to the network thread. The
 * console holds it by shared_ptr, so that it can outlive the server.
 **/
using AdminQueue = engine::MessageQueue<AdminRequest>;

/**
 * @class NetworkServer
 * @brief The server's network thread: the UDP socket, the sessions, the
 * reliability and the timeouts. It also manages the lobby and the rooms: a
 * new session is in the lobby, and receives the list of the rooms until it
 * enters one, created by its first client and destroyed after the last one.
 *
 * It talks to each game thread only through the queues of its room. Everything
 * else, Rooms included, is touched by the network thread alone once it runs,
 * so it needs no lock. Call start()
 * once, and stop() (or let the destructor do it) at shutdown. The socket is
 * an engine::net::IUdpSocket, so no network library appears here.
 **/
class NetworkServer {
  public:
    /**
     * @brief Takes a bound UDP socket. Nothing runs before start().
     * @param socket The socket, from rtype::network::openUdpSocket() (or a
     * fake one in tests).
     * @param rooms The rooms; stop them after stop().
     * @param access Who may connect.
     **/
    NetworkServer(std::unique_ptr<engine::net::IUdpSocket> socket, Rooms &rooms,
                  AccessList access = {});
    NetworkServer(const NetworkServer &) = delete;
    NetworkServer &operator=(const NetworkServer &) = delete;
    NetworkServer(NetworkServer &&) = delete;
    NetworkServer &operator=(NetworkServer &&) = delete;
    /**
     * @brief Stops the network thread if it still runs.
     **/
    ~NetworkServer() { stop(); }

    /**
     * @brief Starts the network thread: receiving datagrams and the update
     * every 10 ms (resends, timeouts, outbound queue).
     **/
    void start();

    /**
     * @brief Returns the commands of the administration console, to add some
     * before start().
     * @return The console, which the network thread runs.
     **/
    [[nodiscard]] engine::CommandConsole &commands() { return m_commands; }

    /**
     * @brief Returns the queue that gives commands to the network thread.
     * @return The queue; see submitCommand().
     **/
    [[nodiscard]] std::shared_ptr<AdminQueue> adminQueue() const
    {
        return m_adminQueue;
    }

    /**
     * @brief Sends Disconnect{ServerClosing} to every client from the
     * network thread, stops it and waits for it. Safe to call twice.
     **/
    void stop();

  private:
    std::unique_ptr<engine::net::IUdpSocket> m_socket;
    Rooms &m_rooms;
    std::thread m_serverThread;
    std::atomic<bool> m_running{false};
    std::array<std::uint8_t, rtype::protocol::MaxPacketSize> m_buffer{};
    std::map<engine::net::Endpoint, Session> m_sessions;
    std::uint32_t m_newCliendId = 1;
    AccessList m_access;
    engine::CommandConsole m_commands;
    std::shared_ptr<AdminQueue> m_adminQueue = std::make_shared<AdminQueue>();
    TimePoint m_startTime;
    TimePoint m_nextRoomList;

    void run();
    void handleDatagram(const engine::net::Datagram &datagram);
    void handleNewPeer(const engine::net::Endpoint &endpoint, TimePoint now);
    void joinRoom(SessionIterator it, engine::InstanceId room, TimePoint now);
    void leaveRoom(SessionIterator it,
                   rtype::protocol::DisconnectReason reason);
    void sendRoomList(SessionIterator it, TimePoint now);
    void sendRoomLists(TimePoint now);
    void handleSessionPacket(SessionIterator it,
                             const rtype::protocol::Packet &packet,
                             TimePoint now);
    void send(SessionIterator it, const rtype::protocol::Payload &payload,
              TimePoint now);
    void sendPacket(const engine::net::Endpoint &endpoint,
                    const rtype::protocol::Packet &packet);
    static void pushInbound(const Session &session, InboundMessage message);
    void closeFailedRoom(engine::InstanceId room);
    void kick(SessionIterator it);
    void registerCommands();
    void runAdminRequests();
    [[nodiscard]] std::string statusCommand() const;
    [[nodiscard]] std::string roomsCommand() const;
    [[nodiscard]] std::string
    playersCommand(const engine::CommandConsole::Args &args) const;
    [[nodiscard]] std::string
    kickCommand(const engine::CommandConsole::Args &args);
    [[nodiscard]] std::string
    banCommand(const engine::CommandConsole::Args &args);
    [[nodiscard]] std::string
    unbanCommand(const engine::CommandConsole::Args &args);
    [[nodiscard]] std::string banlistCommand() const;
    [[nodiscard]] std::string
    stopRoomCommand(const engine::CommandConsole::Args &args);
    [[nodiscard]] std::string
    whitelistCommand(const engine::CommandConsole::Args &args);
    void removeSession(SessionIterator it,
                       rtype::protocol::DisconnectReason reason);
    void update(TimePoint now);
    void closeSessions();
    static bool isRecipient(const OutboundMessage &message,
                            std::uint32_t clientId);
};
/**
 * @brief Asks the network thread to run a console command. Thread-safe.
 * @param queue The queue of the server.
 * @param line The command.
 * @return The future answer; it never comes if the server has stopped.
 **/
[[nodiscard]] std::future<std::string> submitCommand(AdminQueue &queue,
                                                     std::string line);

} // namespace server

#endif // SERVER_NETWORKSERVER_HPP
