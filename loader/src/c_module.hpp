#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>

#include "orionerror.h"

struct ManualMappingData {
  HMODULE(WINAPI *pLoadLibraryA)(LPCSTR);
  FARPROC(WINAPI *pGetProcAddress)(HMODULE, LPCSTR);
  BOOLEAN(WINAPI *pRtlAddFunctionTable)(PRUNTIME_FUNCTION, DWORD, DWORD64);
  BYTE *pBase;
  char token[256];
  char app_id[64];
  void(WINAPI *pReportStage)(const char *, int);
  void(WINAPI *pReportFinished)(BOOL);
};

class c_module {
public:
  static c_module &instance();

  bool init(ManualMappingData *pData);

  static void set_stage(const std::string &name, int progress);
  static std::string get_stage_name();
  static int get_stage_progress();
  static bool is_finished();
  static void set_finished(bool finished);

  ManualMappingData *get_mapping_data() const { return m_data; }

private:
  c_module() = default;
  ~c_module() = default;

  bool run(const std::string &token, const std::string &app_id);

  ManualMappingData *m_data{nullptr};
};
