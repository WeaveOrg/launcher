#pragma once

#include "result.hpp"
#include "protocol.hpp"
#include "tcp_socket.hpp"
#include "binary_stream.hpp"
#include <string>
#include <vector>
#include <atomic>
#include <type_traits>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
using thread_handle_t = HANDLE;
#else
#include <pthread.h>
#include <unistd.h>
using thread_handle_t = pthread_t;
#endif

namespace secure_proto {

// Zero-overhead atomic spinlock (No CRT mutex, No TLS, No OS heap dependencies)
class SpinLock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
public:
    void lock() noexcept {
        while (flag_.test_and_set(std::memory_order_acquire)) {
#if defined(_WIN32)
            YieldProcessor();
#endif
        }
    }
    void unlock() noexcept {
        flag_.clear(std::memory_order_release);
    }
};

class SpinLockGuard {
    SpinLock& lock_;
public:
    explicit SpinLockGuard(SpinLock& lock) noexcept : lock_(lock) { lock_.lock(); }
    ~SpinLockGuard() noexcept { lock_.unlock(); }
    SpinLockGuard(const SpinLockGuard&) = delete;
    SpinLockGuard& operator=(const SpinLockGuard&) = delete;
};

class SecureClient {
public:
    SecureClient(std::string host, uint16_t port, const std::string& server_public_key_hex) noexcept;
    ~SecureClient() noexcept;

    // Non-copyable
    SecureClient(const SecureClient&) = delete;
    SecureClient& operator=(const SecureClient&) = delete;

    // Moveable
    SecureClient(SecureClient&&) noexcept;
    SecureClient& operator=(SecureClient&&) noexcept;

    NetResult connect(uint32_t timeout_ms = 5000) noexcept;
    void disconnect() noexcept;
    bool is_connected() const noexcept { return is_connected_ && socket_.is_valid(); }

    // Heartbeat / Keep-Alive API
    NetResult ping(uint32_t timeout_ms = 5000) noexcept;
    NetResult start_heartbeat(uint32_t interval_ms = 25000) noexcept;
    void stop_heartbeat() noexcept;
    bool is_heartbeat_running() const noexcept { return heartbeat_running_.load(std::memory_order_relaxed); }
    uint32_t heartbeat_interval_ms() const noexcept { return heartbeat_interval_ms_; }

    // High-level API for sending requests (JSON/text or Binary bytecode)
    NetResult get(const std::string& path, Response* out_response, const std::string& headers = "") noexcept;
    NetResult post(const std::string& path, const std::string& json_body, Response* out_response, const std::string& headers = "") noexcept;
    NetResult post_binary(const std::string& path, const std::vector<uint8_t>& binary_payload, Response* out_response, const std::string& headers = "") noexcept;
    NetResult post_binary(const std::string& path, const BinaryWriter& writer, Response* out_response, const std::string& headers = "") noexcept;

    // Zero-Boilerplate Struct Overload: Accepts ANY plain C++ struct (0 Macros, 0 Base classes)
    template <typename T>
    typename std::enable_if_t<std::is_aggregate_v<T>, NetResult>
    post_binary(const std::string& path, const T& object, Response* out_response, const std::string& headers = "") noexcept {
        BinaryWriter writer;
        serialize_struct(writer, object);
        return post_binary(path, writer.buffer(), out_response, headers);
    }

    // Zero-Boilerplate Struct Response Unpacker: Unpacks response into ANY plain C++ struct
    template <typename T>
    static typename std::enable_if_t<std::is_aggregate_v<T>, bool>
    unpack_binary_response(const Response& resp, T& out_obj) noexcept {
        if (resp.is_binary && !resp.raw_body.empty()) {
            BinaryReader reader(resp.raw_body.data(), resp.raw_body.size());
            return deserialize_struct(reader, out_obj);
        }
        if (resp.body.empty()) return false;
        std::vector<uint8_t> raw = crypto::base64_decode(resp.body);
        if (raw.empty()) {
            BinaryReader reader(reinterpret_cast<const uint8_t*>(resp.body.data()), resp.body.size());
            return deserialize_struct(reader, out_obj);
        }
        BinaryReader reader(raw.data(), raw.size());
        return deserialize_struct(reader, out_obj);
    }

    NetResult execute(const Request& request, Response* out_response) noexcept;
    NetResult execute_with_retry(const Request& request, Response* out_response, uint32_t max_retries = 3) noexcept;

private:
    NetResult perform_handshake() noexcept;
    NetResult send_raw_frame_unlocked(const uint8_t* payload, size_t len) noexcept;
    NetResult receive_raw_frame_unlocked(std::vector<uint8_t>& out_payload) noexcept;

    std::string host_;
    uint16_t port_;
    uint8_t server_static_pub_[32];
    bool has_valid_server_key_{false};

    TcpSocket socket_;
    SessionKeys session_{};
    std::atomic<bool> is_connected_{false};
    uint32_t next_req_id_{1};

    mutable SpinLock io_lock_;
    thread_handle_t heartbeat_thread_{};
    std::atomic<bool> heartbeat_running_{false};
    uint32_t heartbeat_interval_ms_{25000};
};

} // namespace secure_proto
