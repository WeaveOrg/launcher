#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace http2client {

enum class TransportMode {
  kTls,
  kPlaintext,
};

// Application protocol spoken over the transport selected by TransportMode.
enum class HttpProtocol {
  // HTTP/2 only: ALPN must negotiate h2 over TLS, and plaintext connections
  // use h2c prior knowledge. This is the default and the only mode that
  // supports gRPC.
  kHttp2,
  // HTTP/1.1 only. Over TLS, ALPN offers http/1.1 (and is skipped entirely
  // when TlsOptions::enable_alpn is false, for servers that do not implement
  // it). gRPC calls are rejected with kInvalidArgument.
  kHttp11,
  // Negotiate: over TLS, ALPN picks h2 when the server offers it and falls
  // back to HTTP/1.1 otherwise. Plaintext connections use HTTP/1.1, because
  // h2c prior knowledge cannot be probed without breaking HTTP/1.1 servers.
  kAuto,
};

enum class LogLevel {
  kError,
  kWarn,
  kInfo,
  kDebug,
};

enum class ErrorCode {
  kNone,
  kCancelled,
  kDeadlineExceeded,
  kNetworkError,
  kDnsError,
  kProxyError,
  kTlsError,
  kHttp2Error,
  kGrpcError,
  kCompressionError,
  kProtocolError,
  kRedirectLimitExceeded,
  kStreamLimitExceeded,
  kInvalidArgument,
  kInternalError,
  kResourceExhausted,
  kHttpStatusError,
};

using HeaderList = std::vector<std::pair<std::string, std::string>>;
// The library is built without C++ exception support. All user callbacks and
// policy functions must return normally and must not throw.
using LoggerCallback = std::function<void(LogLevel, std::string_view)>;

// HSTS policy parsed from a Strict-Transport-Security response header.
struct HstsPolicy {
  std::string host;               // Effective host (lowercase).
  std::uint64_t max_age = 0;      // Seconds; 0 = remove policy.
  bool include_subdomains = false;
  bool preload = false;
};

// Callback invoked when an HSTS policy is observed in a response.
// The application should persist the policy and enforce it on future requests.
using HstsCallback = std::function<void(const HstsPolicy&)>;
using Milliseconds = std::uint64_t;

struct Error {
  ErrorCode code = ErrorCode::kNone;
  std::string message;
  int native_code = 0;

  // Diagnostic context
  std::string request_uri;        // scheme://authority/path for HTTP/gRPC
  std::string operation;          // "TLS handshake", "gRPC unary", "DNS lookup"
  std::uint64_t retry_count = 0;  // Number of reconnect attempts before failure
  std::uint64_t elapsed_ms = 0;   // Milliseconds from request start to error
  std::string grpc_status_message; // gRPC status detail message

  [[nodiscard]] bool ok() const noexcept { return code == ErrorCode::kNone; }
  [[nodiscard]] explicit operator bool() const noexcept { return !ok(); }
};

struct TlsOptions {
  bool verify_peer = true;
  bool enable_sni = true;
  bool enable_alpn = true;

  // When both are empty and verify_peer is true, the Windows system
  // certificate stores (ROOT and CA) are loaded automatically.
  std::string ca_file;
  std::string ca_path;

  // Optional mTLS.
  std::string client_cert_file;
  std::string client_key_file;

  // Optional SPKI pinning. Each entry is a Base64-encoded SHA-256 hash of the
  // Subject Public Key Info (SPKI) of an acceptable leaf certificate, without
  // the "sha256/" prefix (e.g. as reported by OpenSSL's -pubkey output or
  // browser certificate viewer).  When non-empty the TLS handshake is
  // rejected after a successful CA validation if the peer's SPKI does not
  // match any pin.  At least one backup pin (from a different certificate
  // authority) is strongly recommended to avoid lockout on certificate
  // rotation.
  std::vector<std::string> pinned_spki_sha256;
};

enum class ProxyMode {
  // Use the Windows per-user proxy configuration (WinINet/IE settings) when
  // one is configured for the target scheme; connect directly otherwise.
  // PAC scripts / WPAD auto-detection are not evaluated.
  kSystemDefault,
  // Always tunnel through host:port below via HTTP CONNECT.
  kManual,
  // Never use a proxy.
  kDisabled,
};

struct ProxyOptions {
  ProxyMode mode = ProxyMode::kSystemDefault;

  // Manual proxy endpoint (kManual). `host` must not include a port; the
  // `port` field is authoritative.
  std::string host;
  std::uint16_t port = 0;

  // Optional credentials for HTTP Basic Proxy-Authorization (applied in both
  // manual and system modes when non-empty).
  std::string username;
  std::string password;

  // Basic authentication over a plaintext proxy connection exposes the
  // credential to the client-to-proxy network. It is rejected unless the
  // caller explicitly accepts that risk.
  bool allow_cleartext_basic_auth = false;

  // Reserved for HTTPS-proxy support. Current clients reject this option
  // instead of silently sending CONNECT or credentials over the wrong
  // transport. A correct implementation requires nested origin TLS over the
  // still-active proxy TLS session.
  bool tls_to_proxy = false;

  // Reserved TLS configuration for tls_to_proxy.
  TlsOptions tls;
};

struct Http2ClientOptions {
  std::string authority;
  TransportMode mode = TransportMode::kTls;

  // Application protocol. Defaults to HTTP/2 so existing clients keep their
  // exact behavior; set kHttp11 or kAuto to talk to HTTP/1.1-only origins.
  HttpProtocol protocol = HttpProtocol::kHttp2;

  TlsOptions tls;
  ProxyOptions proxy;

  std::size_t max_concurrent_streams = 100;
  Milliseconds connect_timeout = 15000;  // TLS handshake timeout in milliseconds.
  Milliseconds io_poll_interval = 20;
  // Base reconnect delay. Attempts back off exponentially from this value
  // (capped at 30s) with jitter so client fleets do not reconnect in lockstep.
  Milliseconds reconnect_interval = 1000;
  std::size_t max_reconnect_attempts = 5;  // Changed from 0 (infinite) to 5 attempts
  std::optional<Milliseconds> reconnect_timeout{30000};

  // HTTP/2 PING keepalive on an established connection. 0 disables. A PING is
  // sent every keepalive_interval ms; a missing ACK within keepalive_timeout ms
  // is treated as a transport failure and triggers the reconnect/retry policy.
  Milliseconds keepalive_interval = 0;
  Milliseconds keepalive_timeout = 10000;

  // Per-connection HTTP/2 receive window (clamped to [64KiB, 2GiB-1]) and TCP
  // socket buffer size (0 = keep the OS default). Lower these when running
  // many concurrent connections to bound per-connection memory.
  std::size_t receive_window_bytes = 64 * 1024 * 1024;
  std::size_t socket_buffer_bytes = 16 * 1024 * 1024;

  // Resource limits for data controlled by a remote peer. A value of 0 is
  // invalid; Create() returns nullptr rather than constructing an unbounded
  // client.
  std::size_t max_response_header_bytes = 64 * 1024;
  std::size_t max_buffered_response_body_bytes = 64 * 1024 * 1024;
  std::size_t max_grpc_message_bytes = 64 * 1024 * 1024;
  std::size_t max_queued_grpc_write_bytes = 64 * 1024 * 1024;

  // Optional default deadline applied to requests that do not set one.
  // Without a deadline a slow-drip server can hold a stream slot indefinitely
  // (up to max_concurrent_streams).  The easy-layer wrappers already default
  // to 300000ms; callers using the raw async API should set this or supply a
  // per-request deadline.
  std::optional<Milliseconds> default_request_deadline;

  // Optional redirect target validator. When set, called for every automatic
  // redirect after the target authority and transport mode are resolved.
  // Return true to allow the redirect, false to abort with kProtocolError.
  // Use for SSRF protection (deny loopback, link-local, RFC1918 targets).
  std::function<bool(std::string_view authority, TransportMode mode)> redirect_target_validator;

  // Optional post-DNS policy for every direct connection candidate. `host` is
  // the dial target and `numeric_address` is a canonical IPv4/IPv6 string.
  // Return false to skip the candidate. With a proxy this validates the proxy
  // address; proxy-side origin DNS remains outside the client's control.
  std::function<bool(
      std::string_view host,
      std::string_view numeric_address)> resolved_address_validator;

  // Optional HSTS callback.  Called when a response contains a
  // Strict-Transport-Security header.  The application should persist the
  // policy and enforce it on future connections to the same host.
  HstsCallback on_hsts_policy;

  // Optional HSTS policy query.  When enforce_hsts is true, the client calls
  // this before each connection to check whether the target host has a stored
  // HSTS policy (max_age > 0).  Return the stored policy, or a default-constructed
  // HstsPolicy with max_age=0 if no policy is stored.
  std::function<HstsPolicy(const std::string& host)> hsts_policy_query;

  // When true, the client automatically rejects plaintext (kPlaintext)
  // connections to hosts with a stored HSTS policy (max_age > 0) and upgrades
  // them to kTls before authority parsing. Requires hsts_policy_query so the
  // application can feed stored policies back on the next connection.
  bool enforce_hsts = false;

  LoggerCallback logger;
};

struct HttpRequest {
  std::string method = "GET";
  std::string path = "/";

  HeaderList headers;
  std::vector<std::uint8_t> body;
  HeaderList trailers;

  bool enable_gzip = false;
  bool auto_redirect = true;
  bool sync = false;

  // Security: opt-outs for the default redirect hardening. Both default to the
  // safe behavior.
  //
  // allow_https_downgrade: when false (default) an automatic redirect from an
  // HTTPS (kTls) origin to an HTTP (kPlaintext) target is refused with
  // kProtocolError instead of silently following it. Set true only if you
  // genuinely need to follow such downgrades.
  //
  // allow_cross_site_credentials: when false (default) the Authorization and
  // Cookie request headers are stripped on any redirect that changes the origin
  // (scheme, host, or port). When true, those headers may be forwarded to a
  // different TLS origin. They are always stripped on a transport downgrade.
  // Prefer redirect_target_validator to allowlist intended destinations.
  bool allow_https_downgrade = false;
  bool allow_cross_site_credentials = false;

  std::size_t max_redirects = 10;
  std::optional<Milliseconds> deadline;
};

struct HttpCallOptions {
  HeaderList headers;
  HeaderList trailers;
  bool enable_gzip = false;
  bool auto_redirect = true;
  bool sync = false;

  // See HttpRequest for the security semantics of these two opt-outs.
  bool allow_https_downgrade = false;
  bool allow_cross_site_credentials = false;

  std::size_t max_redirects = 10;
  std::optional<Milliseconds> deadline;
};

struct HttpResponse {
  int status_code = 0;
  HeaderList headers;
  std::vector<std::uint8_t> body;
  HeaderList trailers;

  [[nodiscard]] std::string_view header(std::string_view name) const noexcept {
    for (const auto& [k, v] : headers) {
      if (AsciiCaseEqual(k, name)) return v;
    }
    return {};
  }
  [[nodiscard]] std::string body_string() const {
    return std::string(body.begin(), body.end());
  }
  [[nodiscard]] int status_class() const noexcept {
    return status_code >= 100 && status_code < 600 ? status_code / 100 : 0;
  }
 private:
  static bool AsciiCaseEqual(std::string_view left, std::string_view right) noexcept {
    if (left.size() != right.size()) return false;
    for (std::size_t i = 0; i < left.size(); ++i) {
      const auto fold = [](unsigned char ch) {
        return static_cast<unsigned char>(ch >= 'A' && ch <= 'Z' ? ch + ('a' - 'A') : ch);
      };
      if (fold(static_cast<unsigned char>(left[i])) != fold(static_cast<unsigned char>(right[i]))) return false;
    }
    return true;
  }
};

struct MultipartPart {
  std::string name;
  std::optional<std::string> filename;
  std::string content_type;
  HeaderList headers;
  std::vector<std::uint8_t> data;
};

class MultipartForm {
 public:
  MultipartForm();
  explicit MultipartForm(std::string boundary);

  MultipartForm& AddText(
      std::string name,
      std::string value,
      std::string content_type = "text/plain; charset=utf-8");
  MultipartForm& AddFile(
      std::string name,
      std::string filename,
      std::vector<std::uint8_t> data,
      std::string content_type = "application/octet-stream");
  MultipartForm& AddPart(MultipartPart part);

  [[nodiscard]] const std::string& boundary() const noexcept;
  [[nodiscard]] const std::vector<MultipartPart>& parts() const noexcept;

 private:
  std::string boundary_;
  std::vector<MultipartPart> parts_;
};

struct GrpcCallOptions {
  std::string path;
  HeaderList metadata;

  std::optional<Milliseconds> deadline;
  bool enable_request_gzip = false;
  // Require the server to select gzip in its grpc-encoding response field.
  // The client advertises gzip support regardless; individual messages may
  // still legally use the uncompressed flag.
  bool expect_gzip_response = false;

  // Applied by blocking streaming wrappers while collecting responses.
  // Async streaming remains callback-driven and does not accumulate messages.
  std::size_t max_buffered_response_messages = 1024;
  std::size_t max_buffered_response_bytes = 64 * 1024 * 1024;
};

struct GrpcMessage {
  bool compressed = false;
  std::vector<std::uint8_t> payload;
};

struct GrpcStatus {
  int code = 0;
  std::string message;
  HeaderList metadata;
  HeaderList trailers;
};

using HttpHeadersCallback = std::function<void(int status_code, const HeaderList& headers)>;
using HttpDataCallback = std::function<void(std::span<const std::uint8_t> chunk)>;
using HttpDownloadProgressCallback =
    std::function<void(std::size_t downloaded, std::optional<std::size_t> total)>;
using HttpTrailersCallback = std::function<void(const HeaderList& trailers)>;
using HttpCompleteCallback = std::function<void(const HttpResponse& response, const Error& error)>;

struct HttpRequestCallbacks {
  HttpHeadersCallback on_headers;
  HttpDataCallback on_data;
  HttpDownloadProgressCallback on_download_progress;
  HttpTrailersCallback on_trailers;
  HttpCompleteCallback on_complete;
};

using GrpcMetadataCallback = std::function<void(const HeaderList& metadata)>;
using GrpcMessageCallback = std::function<void(const GrpcMessage& message)>;
using GrpcCompleteCallback = std::function<void(const GrpcStatus& status, const Error& error)>;

struct GrpcUnaryCallbacks {
  GrpcMetadataCallback on_initial_metadata;
  GrpcMessageCallback on_message;
  GrpcCompleteCallback on_complete;
};

struct GrpcStreamCallbacks {
  GrpcMetadataCallback on_initial_metadata;
  GrpcMessageCallback on_message;
  GrpcCompleteCallback on_complete;
};

class CallHandle {
 public:
  virtual ~CallHandle() = default;
  virtual void Cancel() = 0;
};

class StreamHandle : public CallHandle {
 public:
  ~StreamHandle() override = default;

  virtual bool WriteMessage(const GrpcMessage& message) = 0;
  virtual void WritesDone() = 0;
};

class Http2Client {
 public:
  virtual ~Http2Client() = default;

  // Callback contract: accepted operations invoke callbacks serially on the
  // connection I/O thread. Callback arguments that use std::span are borrowed
  // and remain valid only until that callback returns. Callbacks must not throw,
  // block the I/O thread, or call a synchronous wrapper for this same client.
  static std::shared_ptr<Http2Client> Create(Http2ClientOptions options);

  virtual std::shared_ptr<CallHandle> StartHttpRequest(
      const HttpRequest& request,
      HttpRequestCallbacks callbacks) = 0;

  std::shared_ptr<CallHandle> Request(
      HttpRequest request,
      HttpRequestCallbacks callbacks = {});

  std::shared_ptr<CallHandle> Get(
      std::string path,
      HttpRequestCallbacks callbacks,
      HttpCallOptions options = {});

  std::shared_ptr<CallHandle> Post(
      std::string path,
      std::vector<std::uint8_t> body,
      HttpRequestCallbacks callbacks,
      HttpCallOptions options = {});

  std::shared_ptr<CallHandle> PostText(
      std::string path,
      std::string body,
      HttpRequestCallbacks callbacks,
      HttpCallOptions options = {},
      std::string content_type = "text/plain; charset=utf-8");

  std::shared_ptr<CallHandle> Put(
      std::string path,
      std::vector<std::uint8_t> body,
      HttpRequestCallbacks callbacks,
      HttpCallOptions options = {});

  std::shared_ptr<CallHandle> Patch(
      std::string path,
      std::vector<std::uint8_t> body,
      HttpRequestCallbacks callbacks,
      HttpCallOptions options = {});

  std::shared_ptr<CallHandle> Delete(
      std::string path,
      HttpRequestCallbacks callbacks,
      HttpCallOptions options = {});

  std::shared_ptr<CallHandle> Head(
      std::string path,
      HttpRequestCallbacks callbacks,
      HttpCallOptions options = {});

  std::shared_ptr<CallHandle> Options(
      std::string path,
      HttpRequestCallbacks callbacks,
      HttpCallOptions options = {});

  std::shared_ptr<CallHandle> PostMultipart(
      std::string path,
      const MultipartForm& form,
      HttpRequestCallbacks callbacks,
      HttpCallOptions options = {});

  static Error EncodeMultipartForm(
      const MultipartForm& form,
      std::vector<std::uint8_t>* body,
      std::string* content_type_header);

  virtual std::shared_ptr<CallHandle> StartGrpcUnary(
      const GrpcCallOptions& options,
      const GrpcMessage& request,
      GrpcUnaryCallbacks callbacks) = 0;

  virtual std::shared_ptr<StreamHandle> StartGrpcClientStreaming(
      const GrpcCallOptions& options,
      GrpcStreamCallbacks callbacks) = 0;

  virtual std::shared_ptr<StreamHandle> StartGrpcServerStreaming(
      const GrpcCallOptions& options,
      const GrpcMessage& request,
      GrpcStreamCallbacks callbacks) = 0;

  virtual std::shared_ptr<StreamHandle> StartGrpcBidiStreaming(
      const GrpcCallOptions& options,
      GrpcStreamCallbacks callbacks) = 0;

  virtual void Shutdown() = 0;
};

}  // namespace http2client
