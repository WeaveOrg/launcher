#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <cstdint>

struct ManualMappingData {
  HMODULE(WINAPI *pLoadLibraryA)(LPCSTR);
  FARPROC(WINAPI *pGetProcAddress)(HMODULE, LPCSTR);
  BOOLEAN(WINAPI *pRtlAddFunctionTable)(PRUNTIME_FUNCTION, DWORD, DWORD64);
  BYTE *pBase;
  char token[256];
  char app_id[64];
};

struct AuthVerifyRequest {
  std::string launcher_token{};
  std::string product_id{};
};

struct AuthVerifyResponse {
  uint32_t status{0};
  std::string message{};
  uint64_t expires_at{0};
  std::string avatar{};
};

class c_module {
public:
  static c_module &instance();

  bool init(ManualMappingData *pData);
  bool verify_auth(const std::string &token, const std::string &app_id);

  const AuthVerifyResponse &get_auth_response() const { return m_auth; }
  ManualMappingData *get_mapping_data() const { return m_data; }

private:
  c_module() = default;
  ~c_module() = default;

  ManualMappingData *m_data{nullptr};
  AuthVerifyResponse m_auth{};
};
