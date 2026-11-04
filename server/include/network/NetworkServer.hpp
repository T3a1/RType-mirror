/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** NetworkServer
*/

#ifndef SERVER_NETWORKSERVER_HPP
#define SERVER_NETWORKSERVER_HPP
#include "common/protocol/ByteWriter.hpp"
#include "common/protocol/Connection.hpp"
#include "common/protocol/Packet.hpp"
#include "common/protocol/ReliableChannel.hpp"
#include "engine/MessageQueue.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/network/IUdpSocket.hpp"
#include "network/NetworkMessages.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
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
     **/
    NetworkServer(std::unique_ptr<engine::net::IUdpSocket> socket, Rooms &rooms)
        : m_socket(std::move(socket)), m_rooms(rooms)
    {
    }
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
    void removeSession(SessionIterator it,
                       rtype::protocol::DisconnectReason reason);
    void update(TimePoint now);
    void closeSessions();
    static bool isRecipient(const OutboundMessage &message,
                            std::uint32_t clientId);
};
} // namespace server

#endif // SERVER_NETWORKSERVER_HPP
