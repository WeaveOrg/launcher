#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <string_view>
#include <thread>
#include <utility>

#include <http2client/http2client_easy.h>

namespace loader {

// Adds request-level retries on top of http2client's connection-level
// reconnect policy. Retries are limited to transient failures so an invalid
// token or a missing file is reported immediately.
class RetryingHttpDownloadClient {
 public:
  using Duration = std::chrono::milliseconds;
  using ProgressCallback = http2client::EasyDownloadProgressCallback;
  using Request = std::function<http2client::EasyResponse(
      std::string_view, ProgressCallback)>;
  using Sleeper = std::function<void(Duration)>;

  struct Options {
    std::size_t max_attempts = 3;
    Duration retry_delay{500};
  };

  explicit RetryingHttpDownloadClient(
      Request request,
      Options options = Options{3, Duration{500}},
      Sleeper sleeper = [](Duration delay) {
        std::this_thread::sleep_for(delay);
      })
      : request_(std::move(request)),
        options_(options),
        sleeper_(std::move(sleeper)) {}

  http2client::EasyResponse GetWithProgress(
      std::string_view path,
      ProgressCallback progress_callback) const {
    const std::size_t max_attempts = (std::max)(std::size_t{1}, options_.max_attempts);
    http2client::EasyResponse response;

    for (std::size_t attempt = 0; attempt < max_attempts; ++attempt) {
      response = request_(path, progress_callback);
      if (!IsRetryable(response) || attempt + 1 == max_attempts)
        return response;

      sleeper_(options_.retry_delay);
    }

    return response;
  }

  static bool IsRetryable(const http2client::EasyResponse& response) noexcept {
    if (!response.error.ok())
      return true;

    if (response.status == 408 || response.status == 429)
      return true;

    if (response.status >= 500 && response.status <= 599)
      return true;

    // The launcher treats an empty successful response as a failed download.
    return response.status >= 200 && response.status < 300 && response.body.empty();
  }

 private:
  Request request_;
  Options options_;
  Sleeper sleeper_;
};

}  // namespace loader
