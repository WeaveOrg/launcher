#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

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
  std::string username{};
};

// Запрос на скачивание
struct InjectDownloadReq {
  std::string launcher_token{};
  std::string product_id{};
};

// Ответ сервера с байтами DLL
struct InjectDownloadResp {
  uint32_t status{0};          // 200 - OK, 403 - Нет подписки/неверный токен, 404 - Файл не найден в S3
  std::vector<uint8_t> data{}; // Сырые байты .dll файла
};

class c_module {
public:
  static c_module &instance();

  bool init(ManualMappingData *pData);
  bool verify_auth(const std::string &token, const std::string &app_id);

  static void set_stage(const std::string &name, int progress);
  static std::string get_stage_name();
  static int get_stage_progress();
  static bool is_finished();
  static void set_finished(bool finished);

  const AuthVerifyResponse &get_auth_response() const { return m_auth; }
  const std::vector<uint8_t> &get_payload_data() const { return m_payload_data; }
  ManualMappingData *get_mapping_data() const { return m_data; }

private:
  c_module() = default;
  ~c_module() = default;

  ManualMappingData *m_data{nullptr};
  AuthVerifyResponse m_auth{};
  std::vector<uint8_t> m_payload_data{};
};
