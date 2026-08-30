#pragma once

#include "http2client.h"
#include "http2client_sync.h"

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

namespace http2client {

// ─── Type Aliases ─────────────────────────────────────────────────────────────

// Callback for download progress: (bytes_downloaded, optional<total_size>)
using EasyDownloadProgressCallback = 
    std::function<void(std::size_t downloaded, std::optional<std::size_t> total)>;

// ─── EasyResponse ─────────────────────────────────────────────────────────────
//
// Returned by every EasyClient method. Check ok() before using body.

struct EasyResponse {
  int        status = 0;
  std::string body;
  HeaderList  headers;
  Error       error;
  HeaderList  trailers;

  [[nodiscard]] bool ok() const noexcept {
    return error.ok() && status >= 200 && status < 300;
  }

  [[nodiscard]] std::string_view text() const noexcept { return body; }

  // Header names are case-insensitive. The returned view is borrowed from this
  // response and remains valid until the response is modified or destroyed.
  [[nodiscard]] std::string_view header(std::string_view name) const noexcept {
    for (const auto& [k, v] : headers) {
      if (AsciiCaseEqual(k, name)) return v;
    }
    return {};
  }

  [[nodiscard]] std::string_view trailer(std::string_view name) const noexcept {
    for (const auto& [k, v] : trailers) {
      if (AsciiCaseEqual(k, name)) return v;
    }
    return {};
  }

 private:
  static bool AsciiCaseEqual(std::string_view left, std::string_view right) noexcept {
    if (left.size() != right.size()) {
      return false;
    }
    for (std::size_t i = 0; i < left.size(); ++i) {
      const auto fold = [](unsigned char ch) {
        return static_cast<unsigned char>(
            ch >= 'A' && ch <= 'Z' ? ch + ('a' - 'A') : ch);
      };
      if (fold(static_cast<unsigned char>(left[i])) !=
          fold(static_cast<unsigned char>(right[i]))) {
        return false;
      }
    }
    return true;
  }
};

// ─── EasyClient ───────────────────────────────────────────────────────────────
//
// Thin high-level wrapper around Http2ClientSync. One or two lines per request:
//
//   EasyClient http("https://api.example.com");
//   auto r = http.Get("/users");
//   if (r.ok()) { process(r.body); }
//
//   http.Bearer("token");
//   auto r = http.Post("/items", R"({"name":"x"})");
//
// Default headers set via Header() / Bearer() / Timeout() persist across all
// calls on this client instance.

class EasyClient {
 public:
  // url: "https://host[:port]", "http://host[:port]", or bare "host[:port]" (TLS).
  explicit EasyClient(std::string_view url)
      : EasyClient(url, Http2ClientOptions{}) {}

  // Parse authority and transport mode from url while retaining every other
  // low-level option (TLS pins, proxy, limits, HSTS, logging, keepalive, etc.).
  EasyClient(std::string_view url, Http2ClientOptions opts) {
    auto [authority, mode] = ParseUrl(url);
    opts.authority = std::move(authority);
    opts.mode      = mode;
    Init(std::move(opts));
  }

  explicit EasyClient(Http2ClientOptions opts) { Init(std::move(opts)); }

  // ── Fluent defaults (mutate this client, thread-safe) ────────────────────

  EasyClient& Header(std::string name, std::string value) {
    std::lock_guard<std::mutex> lock(mutex_);
    call_opts_.headers.emplace_back(std::move(name), std::move(value));
    return *this;
  }

  EasyClient& Bearer(std::string token) {
    return Header("authorization", "Bearer " + std::move(token));
  }

  EasyClient& Timeout(Milliseconds ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    call_opts_.deadline = ms;
    return *this;
  }

  // Replace all persistent per-call defaults at once. This exposes trailers,
  // gzip, redirect policy, redirect limits, headers, and deadlines without
  // growing a separate fluent setter for every low-level option.
  EasyClient& DefaultCallOptions(HttpCallOptions options) {
    std::lock_guard<std::mutex> lock(mutex_);
    options.sync = false;
    call_opts_ = std::move(options);
    return *this;
  }

  // Set callback for download progress tracking. Called with (bytes_downloaded, optional<total_size>).
  // Note: progress_callback is called from the async I/O thread, ensure thread-safety.
  EasyClient& OnDownloadProgress(EasyDownloadProgressCallback progress_callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    progress_callback_ = std::move(progress_callback);
    return *this;
  }

  // ── HTTP methods ──────────────────────────────────────────────────────────

  EasyResponse Get(std::string_view path, HeaderList extra = {}) {
    EasyDownloadProgressCallback progress_callback;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      progress_callback = progress_callback_;
    }
    if (progress_callback) {
      return GetWithProgressInternal(
          path, std::move(progress_callback), std::move(extra));
    }
    Error err;
    auto r = sync_->Get(std::string(path), MakeOpts(std::move(extra)), &err);
    return Wrap(std::move(r), std::move(err));
  }

  // Get with download progress callback
  EasyResponse GetWithProgress(
      std::string_view path,
      EasyDownloadProgressCallback progress_callback,
      HeaderList extra = {}) {
    return GetWithProgressInternal(path, std::move(progress_callback), std::move(extra));
  }

  EasyResponse Post(
      std::string_view path,
      std::string_view body,
      std::string_view content_type = "application/json",
      HeaderList extra = {}) {
    Error err;
    auto r = sync_->PostText(
        std::string(path), std::string(body),
        MakeOpts(std::move(extra)), std::string(content_type), &err);
    return Wrap(std::move(r), std::move(err));
  }

  // QUERY: like Post(), but semantically safe/idempotent — the request carries a
  // body (typically a query expression) yet the server MUST treat it like a
  // retrieval, not a state-changing operation. See RFC 10008.
  EasyResponse Query(
      std::string_view path,
      std::string_view body,
      std::string_view content_type = "application/json",
      HeaderList extra = {}) {
    Error err;
    auto r = sync_->QueryText(
        std::string(path), std::string(body),
        MakeOpts(std::move(extra)), std::string(content_type), &err);
    return Wrap(std::move(r), std::move(err));
  }

  EasyResponse Put(
      std::string_view path, std::string_view body,
      std::string_view content_type = "application/json",
      HeaderList extra = {}) {
    return BodyMethod("PUT", path, body, content_type, std::move(extra));
  }

  EasyResponse Patch(
      std::string_view path, std::string_view body,
      std::string_view content_type = "application/json",
      HeaderList extra = {}) {
    return BodyMethod("PATCH", path, body, content_type, std::move(extra));
  }

  EasyResponse Delete(std::string_view path, HeaderList extra = {}) {
    return BodyMethod("DELETE", path, {}, {}, std::move(extra));
  }

  EasyResponse Head(std::string_view path, HeaderList extra = {}) {
    return BodyMethod("HEAD", path, {}, {}, std::move(extra));
  }

  // Exact-request escape hatch that retains EasyResponse conversion. Unlike
  // the convenience methods, this request is not merged with fluent defaults.
  EasyResponse Request(HttpRequest request) {
    request.sync = false;
    Error err;
    auto response = sync_->Request(std::move(request), &err);
    return Wrap(std::move(response), std::move(err));
  }

  // ── Access underlying clients ─────────────────────────────────────────────

  Http2ClientSync& sync_client() noexcept { return *sync_; }
  [[nodiscard]] Http2Client* async_client() noexcept { return async_.get(); }
  [[nodiscard]] const Http2Client* async_client() const noexcept {
    return async_.get();
  }

  [[nodiscard]] bool valid() const noexcept { return static_cast<bool>(async_); }
  [[nodiscard]] const Error& creation_error() const noexcept {
    return creation_error_;
  }

 private:
  std::shared_ptr<Http2Client>     async_;
  std::unique_ptr<Http2ClientSync> sync_;
  HttpCallOptions                  call_opts_;
  EasyDownloadProgressCallback     progress_callback_;
  Error                            creation_error_;
  mutable std::mutex               mutex_;

  void Init(Http2ClientOptions opts) {
    call_opts_.deadline = opts.default_request_deadline.value_or(300000);
    async_ = Http2Client::Create(std::move(opts));
    if (!async_) {
      creation_error_.code = ErrorCode::kInvalidArgument;
      creation_error_.message = "invalid EasyClient URL or options";
    }
    sync_  = std::make_unique<Http2ClientSync>(async_);
  }

  EasyResponse GetWithProgressInternal(
      std::string_view path,
      EasyDownloadProgressCallback progress_callback,
      HeaderList extra);

  static std::pair<std::string, TransportMode> ParseUrl(std::string_view url) {
    TransportMode mode = TransportMode::kTls;
    const auto starts_with_no_case = [](std::string_view value, std::string_view prefix) {
      if (value.size() < prefix.size()) return false;
      for (std::size_t i = 0; i < prefix.size(); ++i) {
        const auto fold = [](unsigned char ch) {
          return static_cast<unsigned char>(
              ch >= 'A' && ch <= 'Z' ? ch + ('a' - 'A') : ch);
        };
        if (fold(static_cast<unsigned char>(value[i])) !=
            fold(static_cast<unsigned char>(prefix[i]))) {
          return false;
        }
      }
      return true;
    };

    if (starts_with_no_case(url, "https://")) {
      url.remove_prefix(8);
      mode = TransportMode::kTls;
    } else if (starts_with_no_case(url, "http://")) {
      url.remove_prefix(7);
      mode = TransportMode::kPlaintext;
    } else if (url.find("://") != std::string_view::npos) {
      return {{}, mode};
    }

    // EasyClient represents one origin, not a base URL. Accept a cosmetic
    // trailing slash, but reject paths instead of silently discarding them.
    if (url.ends_with('/')) {
      url.remove_suffix(1);
    }
    if (url.empty() || url.find_first_of("/?#") != std::string_view::npos) {
      return {{}, mode};
    }
    return {std::string(url), mode};
  }

  HttpCallOptions MakeOpts(HeaderList extra) const {
    std::lock_guard<std::mutex> lock(mutex_);
    HttpCallOptions opts = call_opts_;
    for (auto& h : extra) opts.headers.push_back(std::move(h));
    return opts;
  }

  static bool HasHeader(const HeaderList& headers, std::string_view name) noexcept {
    for (const auto& [header_name, value] : headers) {
      (void)value;
      if (header_name.size() != name.size()) continue;
      bool equal = true;
      for (std::size_t i = 0; i < name.size(); ++i) {
        const auto fold = [](unsigned char ch) {
          return static_cast<unsigned char>(
              ch >= 'A' && ch <= 'Z' ? ch + ('a' - 'A') : ch);
        };
        if (fold(static_cast<unsigned char>(header_name[i])) !=
            fold(static_cast<unsigned char>(name[i]))) {
          equal = false;
          break;
        }
      }
      if (equal) return true;
    }
    return false;
  }

  EasyResponse BodyMethod(
      const char* method,
      std::string_view path,
      std::string_view body,
      std::string_view content_type,
      HeaderList extra) {
    auto opts = MakeOpts(std::move(extra));
    HttpRequest req;
    req.method  = method;
    req.path    = std::string(path);
    req.headers = std::move(opts.headers);
    req.trailers = std::move(opts.trailers);
    req.enable_gzip = opts.enable_gzip;
    req.auto_redirect = opts.auto_redirect;
    req.sync = opts.sync;
    req.allow_https_downgrade = opts.allow_https_downgrade;
    req.allow_cross_site_credentials = opts.allow_cross_site_credentials;
    req.max_redirects = opts.max_redirects;
    req.deadline = opts.deadline;
    if (!body.empty()) {
      if (!content_type.empty() && !HasHeader(req.headers, "content-type"))
        req.headers.emplace_back("content-type", std::string(content_type));
      req.body.assign(body.begin(), body.end());
    }
    Error err;
    auto r = sync_->Request(std::move(req), &err);
    return Wrap(std::move(r), std::move(err));
  }

  static EasyResponse Wrap(HttpResponse r, Error err) {
    EasyResponse result{
        r.status_code,
        std::string(r.body.begin(), r.body.end()),
        std::move(r.headers),
        std::move(err),
        {},
    };
    result.trailers = std::move(r.trailers);
    return result;
  }
};

}  // namespace http2client
