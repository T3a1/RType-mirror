/*
** EPITECH PROJECT, 2026
** r-type_server
** File description:
** NetworkServer
*/

#include "network/NetworkServer.hpp"
#include "engine/Logger.hpp"

namespace server {

namespace {

/** @brief How often the network thread sends and checks what is due. */
constexpr std::chrono::milliseconds UpdateInterval{10};
/** @brief How often the clients in the lobby receive the list of rooms. */
constexpr std::chrono::milliseconds RoomListInterval{500};

} // namespace

NetworkServer::NetworkServer(std::unique_ptr<engine::net::IUdpSocket> socket,
                             Rooms &rooms, AccessList access)
    : m_socket(std::move(socket)), m_rooms(rooms), m_access(std::move(access))
{
    registerCommands();
}

void NetworkServer::start()
{
    m_startTime = std::chrono::steady_clock::now();
    m_running = true;
    m_serverThread = std::thread([this] {
        try {
            run();
        } catch (const std::exception &e) {
            BB_ERROR(e.what());
        }
    });
}

void NetworkServer::run()
{
    auto nextUpdate = std::chrono::steady_clock::now() + UpdateInterval;
    while (m_running) {
        const auto now = std::chrono::steady_clock::now();
        if (now >= nextUpdate) {
            update(now);
            nextUpdate = now + UpdateInterval;
            continue;
        }
        const auto datagram = m_socket->receive(
            m_buffer,
            std::chrono::ceil<std::chrono::milliseconds>(nextUpdate - now));
        if (datagram) {
            handleDatagram(*datagram);
        }
    }
    closeSessions();
}

void NetworkServer::update(TimePoint now)
{
    for (auto it = m_sessions.begin(); it != m_sessions.end();) {
        if (now - it->second.lastHeard > rtype::protocol::PeerTimeout) {
            removeSession(it++, rtype::protocol::DisconnectReason::Timeout);
            continue;
        }
        if (it->second.kicked) {
            removeSession(it++, rtype::protocol::DisconnectReason::Kicked);
            continue;
        }
        for (const auto &packet : it->second.channel.resendsDue(now)) {
            sendPacket(it->first, packet);
        }
        if (it->second.channel.needsAck()) {
            send(it, rtype::protocol::Ack{}, now);
        }
        ++it;
    }
    m_rooms.forEachLink([&](engine::InstanceId room, Rooms::Link &link) {
        for (const OutboundMessage &message : link.outbound.drain()) {
            for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
                if (it->second.room == room &&
                    isRecipient(message, it->second.id)) {
                    send(it, message.payload, now);
                }
            }
        }
    });
    for (const engine::InstanceId room : m_rooms.reap()) {
        closeFailedRoom(room);
    }
    runAdminRequests();
    if (now >= m_nextRoomList) {
        sendRoomLists(now);
        m_nextRoomList = now + RoomListInterval;
    }
}

void NetworkServer::sendRoomList(SessionIterator it, TimePoint now)
{
    rtype::protocol::RoomList list;
    for (const engine::InstanceInfo &info : m_rooms.list()) {
        list.rooms.push_back({
            .room = info.id,
            .players = static_cast<std::uint8_t>(info.members),
            .status = static_cast<rtype::protocol::RoomStatus>(info.status),
        });
    }
    send(it, list, now);
}

void NetworkServer::sendRoomLists(TimePoint now)
{
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        if (it->second.link == nullptr) {
            sendRoomList(it, now);
        }
    }
}

void NetworkServer::closeFailedRoom(engine::InstanceId room)
{
    BB_ERROR("Room " << room << " failed: closing its sessions");
    for (auto it = m_sessions.begin(); it != m_sessions.end();) {
        if (it->second.room != room) {
            ++it;
            continue;
        }
        sendPacket(
            it->first,
            {
                .payload =
                    rtype::protocol::Disconnect{
                        .reason =
                            rtype::protocol::DisconnectReason::ServerClosing,
                    },
            });
        it = m_sessions.erase(it);
    }
}

void NetworkServer::handleDatagram(const engine::net::Datagram &datagram)
{
    BB_DEBUG("Size " << datagram.size << " bytes received from "
                     << datagram.sender.toString());
    const auto now = std::chrono::steady_clock::now();
    const auto result = rtype::protocol::decodePacket(
        {m_buffer.data(), datagram.size}, rtype::protocol::Side::Client);
    const auto *packet = std::get_if<rtype::protocol::Packet>(&result);
    if (packet == nullptr) {
        const auto error = std::get<rtype::protocol::DecodeError>(result);
        BB_DEBUG("Received wrong datagram "
                 << rtype::protocol::toString(error));
        return;
    }
    auto const it = m_sessions.find(datagram.sender);
    if (it == m_sessions.end()) {
        if (std::holds_alternative<rtype::protocol::Connect>(packet->payload)) {
            handleNewPeer(datagram.sender, now);
        }
        return;
    }
    handleSessionPacket(it, *packet, now);
}

void NetworkServer::removeSession(SessionIterator it,
                                  rtype::protocol::DisconnectReason reason)
{
    BB_INFO("Client " << it->second.id << " disconnected");
    leaveRoom(it, reason);
    m_sessions.erase(it);
}

void NetworkServer::send(SessionIterator it,
                         const rtype::protocol::Payload &payload, TimePoint now)
{
    if (it->second.kicked) {
        return;
    }
    rtype::protocol::Packet packet{.payload = payload};
    if (!it->second.channel.prepare(packet, now)) {
        BB_WARN("Client " << it->second.id << " acknowledges nothing");
        sendPacket(
            it->first,
            rtype::protocol::Packet{
                .payload =
                    rtype::protocol::Disconnect{
                        .reason = rtype::protocol::DisconnectReason::Kicked,
                    },
            });
        it->second.kicked = true;
        return;
    }
    sendPacket(it->first, packet);
}

void NetworkServer::sendPacket(const engine::net::Endpoint &endpoint,
                               const rtype::protocol::Packet &packet)
{
    rtype::protocol::ByteWriter writer;
    if (!rtype::protocol::encodePacket(packet, writer)) {
        BB_ERROR("Packet too large to be sent");
        return;
    }
    const std::error_code error =
        m_socket->sendTo(endpoint, {writer.data().data(), writer.size()});
    if (error) {
        BB_DEBUG("Send failed: " << error.message());
    }
}

void NetworkServer::leaveRoom(SessionIterator it,
                              rtype::protocol::DisconnectReason reason)
{
    if (it->second.link == nullptr) {
        return;
    }
    pushInbound(it->second, {
                                .clientId = it->second.id,
                                .kind = InboundKind::Disconnected,
                                .reason = reason,
                            });
    m_rooms.leave(it->second.room);
    it->second.room = engine::AnyInstance;
    it->second.link = nullptr;
}

void NetworkServer::joinRoom(SessionIterator it, engine::InstanceId room,
                             TimePoint now)
{
    if (it->second.link != nullptr) {
        return;
    }
    const auto membership = room == rtype::protocol::NewRoom
                                ? m_rooms.joinNew()
                                : m_rooms.join(room);
    if (!membership) {
        BB_DEBUG("No room for client " << it->second.id);
        send(it, rtype::protocol::RoomRefused{}, now);
        return;
    }
    it->second.room = membership->id;
    it->second.link = membership->link;
    BB_INFO("Client " << it->second.id << " joined room " << membership->id);
    send(it, rtype::protocol::RoomJoined{.room = membership->id}, now);
    pushInbound(it->second,
                {.clientId = it->second.id, .kind = InboundKind::Connected});
}

void NetworkServer::pushInbound(const Session &session, InboundMessage message)
{
    if (session.link == nullptr) {
        return;
    }
    if (!session.link->inbound.push(message)) {
        BB_ERROR("Could not push event to inbound queue");
    }
}

void NetworkServer::handleNewPeer(const engine::net::Endpoint &endpoint,
                                  TimePoint now)
{
    if (const auto refused = m_access.check(endpoint.address)) {
        BB_INFO("Refused " << endpoint.toString());
        sendPacket(endpoint, {.payload = rtype::protocol::Reject{
                                  .reason = *refused,
                              }});
        return;
    }
    if (m_sessions.size() >= MaxSessions) {
        BB_DEBUG("Too many clients");
        sendPacket(
            endpoint,
            rtype::protocol::Packet{
                .payload =
                    rtype::protocol::Reject{
                        .reason = rtype::protocol::RejectReason::ServerFull,
                    },
            });
        return;
    }
    const auto [it, inserted] = m_sessions.try_emplace(
        endpoint, Session{.id = m_newCliendId++, .lastHeard = now});
    BB_INFO("Registered new client " << it->second.id);
    send(it, rtype::protocol::Accept{}, now);
    sendRoomList(it, now);
}

void NetworkServer::handleSessionPacket(SessionIterator it,
                                        const rtype::protocol::Packet &packet,
                                        TimePoint now)
{
    it->second.lastHeard = now;
    if (!it->second.channel.receive(packet.header)) {
        return;
    }
    if (const auto *input =
            std::get_if<rtype::protocol::Input>(&packet.payload)) {
        pushInbound(it->second, {
                                    .clientId = it->second.id,
                                    .kind = InboundKind::Input,
                                    .sequence = input->sequence,
                                    .actions = input->actions,
                                });
    } else if (const auto *join =
                   std::get_if<rtype::protocol::JoinRoom>(&packet.payload)) {
        joinRoom(it, join->room, now);
    } else if (std::holds_alternative<rtype::protocol::LeaveRoom>(
                   packet.payload)) {
        leaveRoom(it, rtype::protocol::DisconnectReason::Left);
        sendRoomList(it, now);
    } else if (std::holds_alternative<rtype::protocol::StartGame>(
                   packet.payload)) {
        pushInbound(it->second,
                    {.clientId = it->second.id, .kind = InboundKind::Start});
    } else if (std::holds_alternative<rtype::protocol::Disconnect>(
                   packet.payload)) {
        removeSession(it, rtype::protocol::DisconnectReason::Left);
    }
}

void NetworkServer::closeSessions()
{
    for (const auto &[endpoint, session] : m_sessions) {
        sendPacket(
            endpoint,
            {
                .payload =
                    rtype::protocol::Disconnect{
                        .reason =
                            rtype::protocol::DisconnectReason::ServerClosing,
                    },
            });
    }
}

void NetworkServer::stop()
{
    m_running = false;
    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }
}

bool NetworkServer::isRecipient(const OutboundMessage &message,
                                std::uint32_t clientId)
{
    switch (message.to) {
    case Recipient::All:
        return true;
    case Recipient::One:
        return message.cliendId == clientId;
    case Recipient::AllExcept:
        return message.cliendId != clientId;
    }
    return false;
}

} // namespace server
