#pragma once

#include <cstdint>
#include <string>
#include <utility>

namespace secure_proto {

enum class ErrorCode : uint32_t {
    Success = 0,
    InvalidArguments,
    SocketInitFailed,
    ConnectionFailed,
    ConnectionClosed,
    Timeout,
    SendFailed,
    ReceiveFailed,
    HandshakeFailed,
    CryptoError,
    AuthenticationFailed,
    DecryptionFailed,
    ReplayDetected,
    ProtocolError,
    BufferOverflow,
    ServerInternalError,
    RouteNotFound,
    BadRequest
};

inline const char* error_code_to_string(ErrorCode code) noexcept {
    switch (code) {
        case ErrorCode::Success: return "Success";
        case ErrorCode::InvalidArguments: return "Invalid arguments";
        case ErrorCode::SocketInitFailed: return "Failed to initialize socket system";
        case ErrorCode::ConnectionFailed: return "Failed to connect to server";
        case ErrorCode::ConnectionClosed: return "Connection closed by remote peer";
        case ErrorCode::Timeout: return "Operation timed out";
        case ErrorCode::SendFailed: return "Failed to send data over socket";
        case ErrorCode::ReceiveFailed: return "Failed to receive data from socket";
        case ErrorCode::HandshakeFailed: return "Secure handshake failed";
        case ErrorCode::CryptoError: return "Cryptographic operation error";
        case ErrorCode::AuthenticationFailed: return "Authentication / MAC verification failed";
        case ErrorCode::DecryptionFailed: return "Payload decryption failed";
        case ErrorCode::ReplayDetected: return "Replay attack detected / invalid sequence";
        case ErrorCode::ProtocolError: return "Malformed protocol frame";
        case ErrorCode::BufferOverflow: return "Payload exceeded maximum allowed buffer size";
        case ErrorCode::ServerInternalError: return "Server returned internal error";
        case ErrorCode::RouteNotFound: return "Requested route was not found on server";
        case ErrorCode::BadRequest: return "Bad request";
        default: return "Unknown error";
    }
}

class NetResult {
public:
    constexpr NetResult() noexcept : code_(ErrorCode::Success), custom_msg_(nullptr) {}
    constexpr NetResult(ErrorCode code) noexcept : code_(code), custom_msg_(nullptr) {}
    NetResult(ErrorCode code, const char* msg) noexcept : code_(code), custom_msg_(msg) {}

    bool ok() const noexcept { return code_ == ErrorCode::Success; }
    ErrorCode code() const noexcept { return code_; }
    const char* message() const noexcept {
        if (custom_msg_) return custom_msg_;
        return error_code_to_string(code_);
    }

    static constexpr NetResult Ok() noexcept { return NetResult(ErrorCode::Success); }
    static constexpr NetResult Error(ErrorCode code) noexcept { return NetResult(code); }
    static NetResult Error(ErrorCode code, const char* msg) noexcept { return NetResult(code, msg); }

private:
    ErrorCode code_;
    const char* custom_msg_;
};

template <typename T>
class Result {
public:
    Result(const T& val) noexcept : has_val_(true), val_(val), err_(ErrorCode::Success) {}
    Result(T&& val) noexcept : has_val_(true), val_(std::move(val)), err_(ErrorCode::Success) {}
    Result(ErrorCode err) noexcept : has_val_(false), err_(err) {}
    Result(ErrorCode err, const char* msg) noexcept : has_val_(false), err_(err, msg) {}
    Result(NetResult res) noexcept : has_val_(res.ok()), err_(res) {}

    bool ok() const noexcept { return has_val_ && err_.ok(); }
    ErrorCode code() const noexcept { return err_.code(); }
    const char* message() const noexcept { return err_.message(); }

    T& value() noexcept { return val_; }
    const T& value() const noexcept { return val_; }

    T value_or(T default_val) const noexcept {
        if (ok()) return val_;
        return default_val;
    }

    NetResult to_net_result() const noexcept { return err_; }

private:
    bool has_val_;
    T val_{};
    NetResult err_;
};

} // namespace secure_proto
