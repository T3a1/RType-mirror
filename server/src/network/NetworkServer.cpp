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

constexpr std::chrono::milliseconds UpdateInterval{10};

} // namespace

void NetworkServer::start()
{
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
    for (const OutboundMessage &message : m_outbound.drain()) {
        for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
            if (isRecipient(message, it->second.id)) {
                send(it, message.payload, now);
            }
        }
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
    pushInbound({
        .clientId = it->second.id,
        .kind = InboundKind::Disconnected,
        .reason = reason,
    });
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

void NetworkServer::pushInbound(InboundMessage message)
{
    if (!m_inbound.push(message)) {
        BB_ERROR("Could not push event to inbound queue");
    }
}

void NetworkServer::handleNewPeer(const engine::net::Endpoint &endpoint,
                                  TimePoint now)
{
    if (m_sessions.size() >= rtype::protocol::MaxPlayers) {
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
    pushInbound({.clientId = it->second.id, .kind = InboundKind::Connected});
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
        pushInbound({
            .clientId = it->second.id,
            .kind = InboundKind::Input,
            .sequence = input->sequence,
            .actions = input->actions,
        });
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
