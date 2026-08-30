#pragma once

#include "http2client.h"

#include <condition_variable>
#include <memory>
#include <mutex>

namespace http2client {

/**
 * Synchronous wrapper for Http2Client.
 * Provides blocking versions of all async methods.
 */
class Http2ClientSync {
 public:
  explicit Http2ClientSync(std::shared_ptr<Http2Client> client);
  ~Http2ClientSync() = default;

  // HTTP methods (blocking)
  HttpResponse Get(
      std::string path,
      HttpCallOptions options = {},
      Error* out_error = nullptr);

  HttpResponse Post(
      std::string path,
      std::vector<std::uint8_t> body,
      HttpCallOptions options = {},
      Error* out_error = nullptr);

  HttpResponse PostText(
      std::string path,
      std::string body,
      HttpCallOptions options = {},
      std::string content_type = "text/plain; charset=utf-8",
      Error* out_error = nullptr);

  // QUERY: safe, idempotent method that carries a request body (like GET
  // semantics, POST-shaped transport). See RFC 10008.
  HttpResponse QueryText(
      std::string path,
      std::string body,
      HttpCallOptions options = {},
      std::string content_type = "application/json",
      Error* out_error = nullptr);

  HttpResponse Put(
      std::string path,
      std::vector<std::uint8_t> body,
      HttpCallOptions options = {},
      Error* out_error = nullptr);

  HttpResponse Patch(
      std::string path,
      std::vector<std::uint8_t> body,
      HttpCallOptions options = {},
      Error* out_error = nullptr);

  HttpResponse Delete(
      std::string path,
      HttpCallOptions options = {},
      Error* out_error = nullptr);

  HttpResponse Head(
      std::string path,
      HttpCallOptions options = {},
      Error* out_error = nullptr);

  HttpResponse Options(
      std::string path,
      HttpCallOptions options = {},
      Error* out_error = nullptr);

  HttpResponse PostMultipart(
      std::string path,
      const MultipartForm& form,
      HttpCallOptions options = {},
      Error* out_error = nullptr);

  HttpResponse Request(
      HttpRequest request,
      Error* out_error = nullptr);

  // gRPC unary (blocking)
  GrpcMessage StartGrpcUnary(
      const GrpcCallOptions& options,
      const GrpcMessage& request,
      GrpcStatus* out_status = nullptr,
      Error* out_error = nullptr);

  // gRPC streaming - client sends messages, server responds
  // For synchronous blocking, we collect all responses then return
  std::vector<GrpcMessage> StartGrpcClientStreaming(
      const GrpcCallOptions& options,
      std::vector<GrpcMessage> requests,
      GrpcStatus* out_status = nullptr,
      Error* out_error = nullptr);

  // gRPC streaming - server sends multiple responses
  std::vector<GrpcMessage> StartGrpcServerStreaming(
      const GrpcCallOptions& options,
      const GrpcMessage& request,
      GrpcStatus* out_status = nullptr,
      Error* out_error = nullptr);

  // gRPC bidirectional streaming
  std::vector<GrpcMessage> StartGrpcBidiStreaming(
      const GrpcCallOptions& options,
      std::vector<GrpcMessage> requests,
      GrpcStatus* out_status = nullptr,
      Error* out_error = nullptr);

 private:
  std::shared_ptr<Http2Client> client_;

  // Helper for HTTP requests
  struct HttpState {
    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;
    HttpResponse response;
    Error error;
  };

  // Helper for gRPC unary
  struct GrpcUnaryState {
    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;
    GrpcMessage message;
    GrpcStatus status;
    Error error;
  };

  // Helper for gRPC streaming
  struct GrpcStreamState {
    std::mutex mutex;
    std::condition_variable cv;
    bool done = false;
    std::size_t buffered_bytes = 0;
    std::vector<GrpcMessage> messages;
    GrpcStatus status;
    Error error;
  };

  HttpResponse DoHttpRequest(const HttpRequest& request, Error* out_error);
  GrpcMessage DoGrpcUnary(
      const GrpcCallOptions& options,
      const GrpcMessage& request,
      GrpcStatus* out_status,
      Error* out_error);
  std::vector<GrpcMessage> DoGrpcClientStreaming(
      const GrpcCallOptions& options,
      std::vector<GrpcMessage> requests,
      GrpcStatus* out_status,
      Error* out_error);
  std::vector<GrpcMessage> DoGrpcServerStreaming(
      const GrpcCallOptions& options,
      const GrpcMessage& request,
      GrpcStatus* out_status,
      Error* out_error);
  std::vector<GrpcMessage> DoGrpcBidiStreaming(
      const GrpcCallOptions& options,
      std::vector<GrpcMessage> requests,
      GrpcStatus* out_status,
      Error* out_error);
};

}  // namespace http2client
