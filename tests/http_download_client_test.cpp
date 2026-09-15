#include "http_download_client.hpp"

#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string_view>
#include <vector>

using http2client::EasyResponse;
using loader::RetryingHttpDownloadClient;

namespace {

EasyResponse response(int status, std::string body = {}) {
  EasyResponse result;
  result.status = status;
  result.body = std::move(body);
  return result;
}

void retries_transient_http_failure() {
  int requests = 0;
  std::vector<RetryingHttpDownloadClient::Duration> waits;

  RetryingHttpDownloadClient client(
      [&](std::string_view, http2client::EasyDownloadProgressCallback) {
        ++requests;
        return requests == 1 ? response(503) : response(200, "dll");
      },
      {.max_attempts = 3, .retry_delay = std::chrono::milliseconds(7)},
      [&](RetryingHttpDownloadClient::Duration delay) { waits.push_back(delay); });

  const auto result = client.GetWithProgress("/loader.dll", {});

  assert(result.ok());
  assert(result.body == "dll");
  assert(requests == 2);
  assert(waits.size() == 1);
  assert(waits.front() == std::chrono::milliseconds(7));
}

void does_not_retry_permanent_http_failure() {
  int requests = 0;
  RetryingHttpDownloadClient client(
      [&](std::string_view, http2client::EasyDownloadProgressCallback) {
        ++requests;
        return response(404);
      },
      {.max_attempts = 3, .retry_delay = std::chrono::milliseconds(1)},
      [](RetryingHttpDownloadClient::Duration) {});

  const auto result = client.GetWithProgress("/loader.dll", {});

  assert(!result.ok());
  assert(result.status == 404);
  assert(requests == 1);
}

void stops_after_attempt_limit_for_transport_failure() {
  int requests = 0;
  RetryingHttpDownloadClient client(
      [&](std::string_view, http2client::EasyDownloadProgressCallback) {
        ++requests;
        EasyResponse result;
        result.error.code = http2client::ErrorCode::kNetworkError;
        return result;
      },
      {.max_attempts = 3, .retry_delay = std::chrono::milliseconds(1)},
      [](RetryingHttpDownloadClient::Duration) {});

  const auto result = client.GetWithProgress("/loader.dll", {});

  assert(!result.ok());
  assert(result.error.code == http2client::ErrorCode::kNetworkError);
  assert(requests == 3);
}

}  // namespace

int main() {
  retries_transient_http_failure();
  does_not_retry_permanent_http_failure();
  stops_after_attempt_limit_for_transport_failure();
  std::cout << "http_download_client_test: PASS\n";
}
