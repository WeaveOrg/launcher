#pragma once

#include "result.hpp"
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
using socket_t = SOCKET;
constexpr socket_t INVALID_SOCKET_FD = INVALID_SOCKET;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
using socket_t = int;
constexpr socket_t INVALID_SOCKET_FD = -1;
#endif

namespace secure_proto {

class TcpSocket {
public:
    TcpSocket() noexcept;
    ~TcpSocket() noexcept;

    // Non-copyable
    TcpSocket(const TcpSocket&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;

    // Moveable
    TcpSocket(TcpSocket&& other) noexcept;
    TcpSocket& operator=(TcpSocket&& other) noexcept;

    NetResult connect(const std::string& host, uint16_t port, uint32_t timeout_ms = 5000) noexcept;
    void close() noexcept;

    bool is_valid() const noexcept { return fd_ != INVALID_SOCKET_FD; }

    NetResult send_exact(const uint8_t* buffer, size_t length) noexcept;
    NetResult recv_exact(uint8_t* buffer, size_t length) noexcept;

    NetResult set_timeouts(uint32_t timeout_ms) noexcept;
    NetResult set_nodelay(bool enable) noexcept;
    NetResult enable_keepalive(uint32_t idle_sec = 30, uint32_t interval_sec = 5) noexcept;

    static NetResult init_network_system() noexcept;
    static void cleanup_network_system() noexcept;

private:
    socket_t fd_;
};

} // namespace secure_proto
