#pragma once

#include "http2client.h"

#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace http2client {

// ─── Message types ────────────────────────────────────────────────────────────

enum class WsOpcode {
  kText,
  kBinary,
  kClose,
  kPing,
  kPong,
};

struct WsMessage {
  WsOpcode    type         = WsOpcode::kText;
  std::string data;
  uint16_t    close_code   = 1000;
  std::string close_reason;
};

// ─── Configuration ────────────────────────────────────────────────────────────

struct WsOptions {
  Milliseconds write_timeout = 15000; // Total time allowed to write one frame.
  HeaderList               headers;
  std::vector<std::string> protocols;       // Sec-WebSocket-Protocol values
  TlsOptions               tls;
  // Both ws:// and wss:// are tunnelled through the proxy via HTTP CONNECT.
  ProxyOptions             proxy;
  Milliseconds             connect_timeout = 15000;
  Milliseconds             ping_interval          = 0;  // 0 = disabled
  Milliseconds             pong_timeout           = 0;  // 0 = disabled; time to wait for pong reply before disconnecting
  Milliseconds             io_poll_interval       = 20; // I/O event loop poll interval
  Milliseconds             close_handshake_timeout = 5000; // max wait for server close echo

  // Reconnect policy (applies to both initial connect failures and mid-session drops).
  bool         auto_reconnect         = false;
  Milliseconds reconnect_interval     = 1000;  // delay between attempts
  std::size_t  max_reconnect_attempts = 5;     // 0 = infinite
  Milliseconds reconnect_timeout      = 30000; // 0 = no overall limit

  LoggerCallback logger;

  // Bounds for peer-controlled input and locally queued data. Zero is invalid.
  // Control-frame payloads remain limited to 125 bytes by RFC 6455.
  std::size_t max_handshake_header_bytes = 64 * 1024;
  std::size_t max_message_bytes          = 64 * 1024 * 1024;
  std::size_t max_send_queue_bytes       = 64 * 1024 * 1024;

  // Used by the blocking WsClient receive queue. The callback-based API does
  // not retain received messages after on_message returns.
  std::size_t max_receive_queue_messages = 1024;
  std::size_t max_receive_queue_bytes    = 64 * 1024 * 1024;
};

struct WsCallbacks {
  // Callbacks run serially on the WebSocket I/O thread. The library is built
  // without exceptions, so callbacks must not throw or block for long periods.
  std::function<void()>                                  on_open;
  std::function<void(WsMessage)>                         on_message;
  std::function<void(uint16_t code, std::string reason)> on_close;
  std::function<void(Error)>                             on_error;
  std::function<void(std::string_view)>                  on_pong;
};

// ─── Ready state ─────────────────────────────────────────────────────────────

enum class WsReadyState {
  kConnecting,
  kOpen,
  kClosing,
  kClosed,
};

// ─── Async handle ─────────────────────────────────────────────────────────────

// Returned by WsConnect. Thread-safe: Send/Close may be called from any thread.
class WsHandle {
 public:
  virtual ~WsHandle() = default;

  virtual bool Send(std::string_view text)             = 0;
  virtual bool SendBinary(std::span<const uint8_t> data) = 0;
  virtual bool Ping(std::string_view payload = {})     = 0;
  virtual void Close(uint16_t code = 1000, std::string_view reason = {}) = 0;

  [[nodiscard]] virtual bool is_open() const = 0;

  [[nodiscard]] virtual std::string negotiated_protocol() const = 0;

  [[nodiscard]] virtual std::size_t buffered_amount() const = 0;

  [[nodiscard]] virtual WsReadyState ready_state() const = 0;

  [[nodiscard]] virtual std::string url() const = 0;
};

// Async WebSocket connect. Returns immediately; on_open fires on success,
// on_error fires on connection/setup failure (in either case from a background
// thread). Unless the handle itself is destroyed to cancel the operation, every
// opened session ends with on_close; code 1006 denotes an abnormal transport
// termination and is never sent on the wire.
// url: "ws://host[:port][/path][?query]" or
//      "wss://host[:port][/path][?query]". URI fragments and userinfo are
// rejected.
std::shared_ptr<WsHandle> WsConnect(
    std::string url, WsCallbacks callbacks, WsOptions opts = {});

// ─── Blocking simple client ───────────────────────────────────────────────────
//
// Easiest possible WebSocket usage:
//
//   auto ws = WsClient::Connect("wss://echo.example.com/ws");
//   if (!ws.is_open()) { handle(ws.error()); }
//   ws.Send("hello");
//   auto msg = ws.Receive();   // blocks up to 5 s
//   ws.Close();

class WsClient {
 public:
  // Blocks until the handshake completes or opts.connect_timeout elapses.
  static WsClient Connect(std::string url, WsOptions opts = {});

  [[nodiscard]] const Error& error()   const noexcept;
  [[nodiscard]] bool         is_open() const noexcept;

  bool Send(std::string_view text);
  bool SendBinary(std::span<const uint8_t> data);

  // Block until a message arrives or timeout_ms elapses (0 = wait forever).
  // Returns std::nullopt on timeout; check timed_out() or closed() to learn why.
  std::optional<WsMessage> Receive(Milliseconds timeout_ms = 5000);

  [[nodiscard]] bool closed() const noexcept;
  [[nodiscard]] bool timed_out() const noexcept;

  void Close(uint16_t code = 1000, std::string_view reason = {});

 private:
  struct Impl;
  std::shared_ptr<Impl> impl_;
  explicit WsClient(std::shared_ptr<Impl> impl) : impl_(std::move(impl)) {}
};

}  // namespace http2client
