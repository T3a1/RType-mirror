/*
** EPITECH PROJECT, 2026
** r-type_tests
** File description:
** FakeNetwork
*/

#ifndef TESTS_FAKENETWORK_HPP
#define TESTS_FAKENETWORK_HPP

#include "common/protocol/ByteWriter.hpp"
#include "common/protocol/Packet.hpp"
#include "common/protocol/ReliableChannel.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/network/IUdpSocket.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <stdexcept>
#include <thread>
#include <variant>
#include <vector>

namespace tests {

/**
 * @class FakeNetwork
 * @brief An in-memory, lossless UDP network for the tests. Sockets opened
 * on it exchange datagrams through per-endpoint inboxes; a datagram sent to
 * an endpoint with no socket is dropped. Thread-safe.
 **/
class FakeNetwork {
  public:
    /**
     * @brief Opens a socket bound to an endpoint of this network.
     * @param local The socket's own endpoint.
     * @return The socket. The network must outlive it.
     **/
    std::unique_ptr<engine::net::IUdpSocket>
    open(const engine::net::Endpoint &local);

    /**
     * @brief Queues a datagram in the inbox of its recipient.
     * @param from The sender.
     * @param to The recipient.
     * @param data The bytes.
     **/
    void deliver(const engine::net::Endpoint &from,
                 const engine::net::Endpoint &to,
                 std::span<const std::uint8_t> data)
    {
        {
            const std::scoped_lock lock(m_mutex);
            const auto inbox = m_inboxes.find(to);
            if (inbox == m_inboxes.end()) {
                return;
            }
            inbox->second.push_back({from, {data.begin(), data.end()}});
        }
        m_ready.notify_all();
    }

    /**
     * @brief Waits for the next datagram of an inbox.
     * @param local The inbox.
     * @param buffer Where the datagram is copied, truncated to its size.
     * @param timeout How long to wait.
     * @return The sender and size, or nothing on timeout.
     **/
    std::optional<engine::net::Datagram>
    take(const engine::net::Endpoint &local, std::span<std::uint8_t> buffer,
         std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(m_mutex);
        auto &inbox = m_inboxes[local];
        if (!m_ready.wait_for(lock, timeout,
                              [&inbox] { return !inbox.empty(); })) {
            return std::nullopt;
        }
        const Message message = std::move(inbox.front());
        inbox.pop_front();
        const std::size_t size = std::min(buffer.size(), message.bytes.size());
        std::copy_n(message.bytes.begin(), size, buffer.begin());
        return engine::net::Datagram{.sender = message.from, .size = size};
    }

    /**
     * @brief Binds an endpoint: datagrams sent to it are kept from now on.
     * @param local The endpoint.
     **/
    void bind(const engine::net::Endpoint &local)
    {
        const std::scoped_lock lock(m_mutex);
        m_inboxes[local];
    }

    /**
     * @brief Unbinds an endpoint and drops its pending datagrams.
     * @param local The endpoint.
     **/
    void unbind(const engine::net::Endpoint &local)
    {
        const std::scoped_lock lock(m_mutex);
        m_inboxes.erase(local);
    }

  private:
    struct Message {
        engine::net::Endpoint from;
        std::vector<std::uint8_t> bytes;
    };

    std::mutex m_mutex;
    std::condition_variable m_ready;
    std::map<engine::net::Endpoint, std::deque<Message>> m_inboxes;
};

/**
 * @class FakeUdpSocket
 * @brief An engine::net::IUdpSocket on a FakeNetwork.
 **/
class FakeUdpSocket final : public engine::net::IUdpSocket {
  public:
    FakeUdpSocket(FakeNetwork &network, const engine::net::Endpoint &local)
        : m_network(&network), m_local(local)
    {
        m_network->bind(m_local);
    }
    ~FakeUdpSocket() override { m_network->unbind(m_local); }

    FakeUdpSocket(const FakeUdpSocket &) = delete;
    FakeUdpSocket &operator=(const FakeUdpSocket &) = delete;
    FakeUdpSocket(FakeUdpSocket &&) = delete;
    FakeUdpSocket &operator=(FakeUdpSocket &&) = delete;

    std::error_code sendTo(const engine::net::Endpoint &to,
                           std::span<const std::uint8_t> data) override
    {
        m_network->deliver(m_local, to, data);
        return {};
    }

    std::optional<engine::net::Datagram>
    receive(std::span<std::uint8_t> buffer,
            std::chrono::milliseconds timeout) override
    {
        return m_network->take(m_local, buffer, timeout);
    }

    [[nodiscard]] engine::net::Endpoint localEndpoint() const override
    {
        return m_local;
    }

  private:
    FakeNetwork *m_network;
    engine::net::Endpoint m_local;
};

inline std::unique_ptr<engine::net::IUdpSocket>
FakeNetwork::open(const engine::net::Endpoint &local)
{
    return std::make_unique<FakeUdpSocket>(*this, local);
}

/**
 * @class FakePeer
 * @brief A hand-driven protocol peer on a FakeNetwork: it sends payloads
 * with its own ReliableChannel, or raw bytes, and reads decoded packets.
 **/
class FakePeer {
  public:
    using Clock = std::chrono::steady_clock;

    /**
     * @param network The network to open the socket on.
     * @param self The peer's endpoint.
     * @param remote Where it sends to.
     * @param remoteSide Which side the remote is, to decode its packets.
     **/
    FakePeer(FakeNetwork &network, const engine::net::Endpoint &self,
             const engine::net::Endpoint &remote,
             rtype::protocol::Side remoteSide)
        : m_socket(network.open(self)), m_remote(remote),
          m_remoteSide(remoteSide)
    {
    }

    void send(const rtype::protocol::Payload &payload)
    {
        rtype::protocol::Packet packet{.payload = payload};
        static_cast<void>(m_channel.prepare(packet, Clock::now()));
        rtype::protocol::ByteWriter writer;
        static_cast<void>(rtype::protocol::encodePacket(packet, writer));
        sendRaw({writer.data().data(), writer.size()});
    }

    void sendRaw(std::span<const std::uint8_t> bytes)
    {
        if (m_socket->sendTo(m_remote, bytes)) {
            throw std::runtime_error("The fake network cannot fail");
        }
    }

    /**
     * @brief Reads packets until one carries a T, acknowledging and
     * skipping the others.
     * @param timeout How long to wait in total.
     * @return The payload, or nothing on timeout.
     **/
    template <typename T>
    std::optional<T>
    waitFor(std::chrono::milliseconds timeout = std::chrono::seconds(2))
    {
        const auto deadline = Clock::now() + timeout;
        while (Clock::now() < deadline) {
            const auto datagram =
                m_socket->receive(m_buffer, std::chrono::milliseconds(5));
            if (!datagram) {
                continue;
            }
            const auto result = rtype::protocol::decodePacket(
                {m_buffer.data(), datagram->size}, m_remoteSide);
            const auto *packet = std::get_if<rtype::protocol::Packet>(&result);
            if (packet == nullptr || !m_channel.receive(packet->header)) {
                continue;
            }
            if (const auto *payload = std::get_if<T>(&packet->payload)) {
                return *payload;
            }
        }
        return std::nullopt;
    }

  private:
    std::unique_ptr<engine::net::IUdpSocket> m_socket;
    engine::net::Endpoint m_remote;
    rtype::protocol::Side m_remoteSide;
    rtype::protocol::ReliableChannel m_channel;
    std::array<std::uint8_t, rtype::protocol::MaxPacketSize> m_buffer{};
};

/**
 * @brief Polls a condition until it holds or a timeout expires.
 * @param condition The condition.
 * @param timeout How long to wait.
 * @return Whether the condition held in time.
 **/
inline bool
eventually(const std::function<bool()> &condition,
           std::chrono::milliseconds timeout = std::chrono::seconds(2))
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (condition()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return condition();
}

} // namespace tests

#endif // TESTS_FAKENETWORK_HPP
