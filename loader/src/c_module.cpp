#include "c_module.hpp"
#include <secure_client.hpp>
#include <stdio.h>

c_module &c_module::instance() {
  static c_module inst;
  return inst;
}

bool c_module::init(ManualMappingData *pData) {
  m_data = pData;
  if (!pData)
    return false;

  std::string token = pData->token;
  std::string app_id = pData->app_id;

  return verify_auth(token, app_id);
}

bool c_module::verify_auth(const std::string &token, const std::string &app_id) {
  using namespace secure_proto;

  std::string host = "127.0.0.1";
  uint16_t port = 9055;
  std::string pubkey_hex =
      "07a37cbc142093c8b755dc1b10e86cb426374ad16aa853ed0bdfc0b2b86d1c7c";

  SecureClient client(host, port, pubkey_hex);

  NetResult res = client.connect(5000);
  if (!res.ok()) {
    char err[512];
    sprintf_s(err, sizeof(err), "Failed to connect to auth server: %s",
              res.message());
    MessageBoxA(NULL, err, "Weave Module Error", MB_OK | MB_ICONERROR);
    return false;
  }

  AuthVerifyRequest verify_req;
  verify_req.launcher_token = token;
  verify_req.product_id = app_id;

  Response verify_raw_response;
  res = client.post_binary("/api/v1/auth", verify_req, &verify_raw_response);
  if (!res.ok() || verify_raw_response.status_code != 200) {
    char err[512];
    sprintf_s(err, sizeof(err),
              "Verify Request Failed! Error: %s (Status: %d)", res.message(),
              verify_raw_response.status_code);
    MessageBoxA(NULL, err, "Weave Module Auth Failed",
                MB_OK | MB_ICONERROR);
    return false;
  }

  if (!SecureClient::unpack_binary_response(verify_raw_response, m_auth)) {
    MessageBoxA(NULL, "Failed to unpack auth response payload!",
                "Weave Module Error", MB_OK | MB_ICONERROR);
    return false;
  }

  char msg[1024];
  sprintf_s(msg, sizeof(msg),
            "Module Authenticated Successfully!\n\nMessage: %s\nStatus: "
            "%u\nExpires: %llu\nAvatar: %s",
            m_auth.message.c_str(), m_auth.status,
            (unsigned long long)m_auth.expires_at, m_auth.avatar.c_str());
  MessageBoxA(NULL, msg, "Weave Module Auth", MB_OK | MB_ICONINFORMATION);

  return true;
}
