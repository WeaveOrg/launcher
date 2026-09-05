#undef UNICODE
#undef _UNICODE
#include "loader.hpp"
#include "orionerror.h"
#include <windows.h>
#include <algorithm>
#include <format>
#include <fstream>
#include <http2client/http2client_easy.h>
#include <mutex>
#include <string>
#include <tlhelp32.h>
#include <vector>

#ifdef _DEBUG
#define LAUNCHER_BASE_URL "http://localhost:3000"
#else
#define LAUNCHER_BASE_URL "https://launcher.weave.su"
#endif

#define CDN_URL "https://cdn.weave.su"

namespace loader {

static std::mutex g_stage_mutex;
static std::string g_stage_name = "Ready";
static int g_stage_progress = 0;
static bool g_is_finished = false;
static bool g_was_successful = false;
static OrionError g_last_error = ORION_ERROR_NONE;
static std::string g_last_error_detail;

void set_stage(const std::string &name, int progress) {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  g_stage_name = name;
  g_stage_progress = progress;
}

void set_finished(bool finished) {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  g_is_finished = finished;
}

std::string get_stage_name() {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  return g_stage_name;
}

int get_stage_progress() {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  return g_stage_progress;
}

bool is_finished() {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  return g_is_finished;
}

bool was_successful() {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  return g_was_successful;
}

int get_error_code() {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  return static_cast<int>(g_last_error);
}

std::string get_error_string() {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  if (!g_last_error_detail.empty())
    return g_last_error_detail;
  if (g_last_error == ORION_ERROR_NONE)
    return {};
  return OrionErrorToString(g_last_error);
}

static void WINAPI report_loader_stage(const char *name, int progress) {
  // Bootstrap occupies 0-45%; loader.dll owns the remaining 55%.
  progress = (std::max)(0, (std::min)(100, progress));
  set_stage(name ? name : "Working...", progress);
}

static void WINAPI report_loader_finished(BOOL success) {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  g_was_successful = success != FALSE;
  g_is_finished = true;
  if (g_was_successful)
    g_stage_progress = 100;
}

// Convenience: set error code, update the stage label with exact details, and mark as failed.
static void fail_with_error(OrionError err, const std::string &stage_msg, const std::string &detail = "") {
  std::string full_msg = stage_msg;
  if (!detail.empty()) {
    full_msg += ": " + detail;
  }
  full_msg += " [" + std::to_string(static_cast<int>(err)) + ": " + OrionErrorToString(err) + "]";

  {
    std::lock_guard<std::mutex> lock(g_stage_mutex);
    g_last_error = err;
    g_last_error_detail = full_msg;
  }
  set_stage(full_msg, 0);
  report_loader_finished(FALSE);
}

struct ManualMappingData {
  HMODULE(WINAPI *pLoadLibraryA)(LPCSTR);
  FARPROC(WINAPI *pGetProcAddress)(HMODULE, LPCSTR);
  BOOLEAN(WINAPI *pRtlAddFunctionTable)(PRUNTIME_FUNCTION, DWORD, DWORD64);

  BYTE *pBase;

  // Custom arguments to pass to DllMain
  char token[256];
  char app_id[64];
  void(WINAPI *pReportStage)(const char *, int);
  void(WINAPI *pReportFinished)(BOOL);
};

// The shellcode that will execute in the target process
#pragma runtime_checks("", off)
#pragma optimize("", off)
void __stdcall LoaderShellcode(ManualMappingData *pData) {
  if (!pData)
    return;

  BYTE *pBase = pData->pBase;
  auto *pOpt =
      &reinterpret_cast<IMAGE_NT_HEADERS *>(
           pBase + reinterpret_cast<IMAGE_DOS_HEADER *>(pBase)->e_lfanew)
           ->OptionalHeader;

  auto _LoadLibraryA = pData->pLoadLibraryA;
  auto _GetProcAddress = pData->pGetProcAddress;

  // Resolve Imports
  if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
    auto *pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(
        pBase +
        pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    while (pImportDescr->Name) {
      char *szMod = reinterpret_cast<char *>(pBase + pImportDescr->Name);
      HINSTANCE hDll = _LoadLibraryA(szMod);

      ULONG_PTR *pThunkRef = reinterpret_cast<ULONG_PTR *>(
          pBase + pImportDescr->OriginalFirstThunk);
      ULONG_PTR *pFuncRef =
          reinterpret_cast<ULONG_PTR *>(pBase + pImportDescr->FirstThunk);

      if (!pThunkRef)
        pThunkRef = pFuncRef;

      for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
        if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
          *pFuncRef = (ULONG_PTR)_GetProcAddress(
              hDll, reinterpret_cast<char *>(*pThunkRef & 0xFFFF));
        } else {
          auto *pImport =
              reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(pBase + (*pThunkRef));
          *pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, pImport->Name);
        }
      }
      ++pImportDescr;
    }
  }

  // Fix Relocations
  if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
    auto *pBaseReloc = reinterpret_cast<IMAGE_BASE_RELOCATION *>(
        pBase +
        pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    ULONG_PTR delta = reinterpret_cast<ULONG_PTR>(pBase) - pOpt->ImageBase;

    while (pBaseReloc->VirtualAddress) {
      if (pBaseReloc->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION)) {
        size_t count =
            (pBaseReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
            sizeof(WORD);
        WORD *list = reinterpret_cast<WORD *>(pBaseReloc + 1);

        for (size_t i = 0; i < count; ++i) {
          if (list[i]) {
            WORD type = list[i] >> 12;
            WORD offset = list[i] & 0xFFF;
            if (type == IMAGE_REL_BASED_HIGHLOW ||
                type == IMAGE_REL_BASED_DIR64) {
              auto *pPatch = reinterpret_cast<ULONG_PTR *>(
                  pBase + pBaseReloc->VirtualAddress + offset);
              *pPatch += delta;
            }
          }
        }
      }
      pBaseReloc = reinterpret_cast<IMAGE_BASE_RELOCATION *>(
          reinterpret_cast<BYTE *>(pBaseReloc) + pBaseReloc->SizeOfBlock);
    }
  }

  // Setup SEH for 64-bit binaries
  if (pData->pRtlAddFunctionTable &&
      pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size) {
    auto *pFuncTable = reinterpret_cast<PRUNTIME_FUNCTION>(
        pBase +
        pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress);
    DWORD entryCount =
        pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size /
        sizeof(RUNTIME_FUNCTION);
    pData->pRtlAddFunctionTable(pFuncTable, entryCount, (DWORD64)pBase);
  }

  // Call TLS Callbacks
  if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
    auto *pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY *>(
        pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
    auto *pCallback =
        reinterpret_cast<PIMAGE_TLS_CALLBACK *>(pTLS->AddressOfCallBacks);
    while (pCallback && *pCallback) {
      (*pCallback)(pBase, DLL_PROCESS_ATTACH, nullptr);
      pCallback++;
    }
  }

  // Call Entry Point (passing pData as lpReserved to forward token/auth
  // details)
  if (pOpt->AddressOfEntryPoint) {
    using DllEntry_t = BOOL(WINAPI *)(void *, DWORD, void *);
    auto DllEntry =
        reinterpret_cast<DllEntry_t>(pBase + pOpt->AddressOfEntryPoint);
    DllEntry(pBase, DLL_PROCESS_ATTACH, pData);
  }
}
// Stub to calculate shellcode size
void __stdcall LoaderShellcodeEnd() {}
#pragma optimize("", on)
#pragma runtime_checks("", restore)

DWORD FindTargetPid(const std::string &app) {
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return 0;
  PROCESSENTRY32 pe;
  pe.dwSize = sizeof(PROCESSENTRY32);
  unsigned long pid = 0;
  if (Process32First(hSnap, &pe)) {
    do {
      if (_stricmp(pe.szExeFile, app.c_str()) == 0) {
        pid = pe.th32ProcessID;
        break;
      }
    } while (Process32Next(hSnap, &pe));
  }
  CloseHandle(hSnap);
  return pid;
}

bool fetch_and_inject(const std::string &app_id, const std::string &token) {
  {
    std::lock_guard<std::mutex> lock(g_stage_mutex);
    g_is_finished = false;
    g_was_successful = false;
    g_last_error = ORION_ERROR_NONE;
    g_last_error_detail.clear();
  }

  http2client::Http2ClientOptions client_opts;
  client_opts.protocol = http2client::HttpProtocol::kAuto;
  client_opts.connect_timeout = 15000;
  http2client::EasyClient client(CDN_URL, client_opts);
  client.Bearer(token).Timeout(15000);

  int download_progress = 0;
  auto resp = client.GetWithProgress(
      "/loader.dll",
      [&](std::size_t downloaded, std::optional<std::size_t> total) {
        if (!total.has_value())
          return;
        download_progress =
            static_cast<int>(static_cast<float>(downloaded) /
                             static_cast<float>(total.value()) * 100.f);
        set_stage("Download Library (" + std::to_string(download_progress) +
                      "%)",
                  download_progress);
      });

  if (!resp.ok() || resp.body.empty()) {
    std::string err_desc;
    OrionError err_code = ORION_ERROR_DOWNLOAD_MODULE_1;

    if (!resp.error.ok()) {
      switch (resp.error.code) {
      case http2client::ErrorCode::kTlsError:
        err_code = ORION_ERROR_CREATE_SSL_CLIENT;
        err_desc = "SSL/TLS handshake error";
        break;
      case http2client::ErrorCode::kNetworkError:
        err_code = ORION_ERROR_CONNECT_TO_SERVER;
        err_desc = "TCP/Network connection error";
        break;
      case http2client::ErrorCode::kDnsError:
        err_code = ORION_ERROR_CONNECT_TO_SERVER;
        err_desc = "DNS resolution failed";
        break;
      case http2client::ErrorCode::kDeadlineExceeded:
        err_desc = "Connection timed out (15s limit)";
        break;
      case http2client::ErrorCode::kProxyError:
        err_desc = "Proxy connection error";
        break;
      case http2client::ErrorCode::kHttp2Error:
        err_desc = "HTTP/2 protocol error";
        break;
      case http2client::ErrorCode::kProtocolError:
        err_desc = "Protocol error";
        break;
      case http2client::ErrorCode::kHttpStatusError:
        err_desc = "HTTP status error " + std::to_string(resp.status);
        break;
      default:
        err_desc = "Network error (code " + std::to_string(static_cast<int>(resp.error.code)) + ")";
        break;
      }

      if (!resp.error.operation.empty()) {
        err_desc += " during " + resp.error.operation;
      }
      if (!resp.error.message.empty()) {
        err_desc += " (" + resp.error.message + ")";
      }
      if (resp.error.native_code != 0) {
        err_desc += " [native/WSA: " + std::to_string(resp.error.native_code) + "]";
      }
    } else if (resp.status < 200 || resp.status >= 300) {
      err_desc = "HTTP Error " + std::to_string(resp.status);
      switch (resp.status) {
      case 401: err_desc += " Unauthorized (invalid token)"; break;
      case 403: err_desc += " Forbidden (access denied)"; break;
      case 404: err_desc += " Not Found (loader.dll missing on CDN)"; break;
      case 500: err_desc += " Internal Server Error"; break;
      case 502: err_desc += " Bad Gateway (upstream CDN offline)"; break;
      case 503: err_desc += " Service Unavailable"; break;
      case 504: err_desc += " Gateway Timeout"; break;
      case 520: case 521: case 522: case 523: case 524: case 525:
        err_desc += " Cloudflare origin error"; break;
      default: break;
      }
    } else if (resp.body.empty()) {
      err_desc = "Server returned empty file (0 bytes)";
    }

    fail_with_error(err_code, "Download failed", err_desc);
    return false;
  }

  set_stage("Loading Library", 0);
  Sleep(350);

  std::vector<BYTE> dllData(resp.body.begin(), resp.body.end());
  if (dllData.size() < sizeof(IMAGE_DOS_HEADER)) {
    fail_with_error(ORION_ERROR_INVALID_MODULE_1, "Invalid PE file size",
                    "Received " + std::to_string(dllData.size()) + " bytes, expected >= " +
                        std::to_string(sizeof(IMAGE_DOS_HEADER)));
    return false;
  }

  IMAGE_DOS_HEADER *pDos = reinterpret_cast<IMAGE_DOS_HEADER *>(dllData.data());
  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) {
    bool is_text = true;
    for (size_t i = 0; i < (std::min)(dllData.size(), size_t(128)); ++i) {
      if (dllData[i] == 0 || (dllData[i] < 32 && dllData[i] != '\r' && dllData[i] != '\n' && dllData[i] != '\t')) {
        is_text = false;
        break;
      }
    }
    std::string preview;
    if (is_text) {
      preview = "Server returned text/HTML instead of binary: " +
                std::string(reinterpret_cast<char *>(dllData.data()),
                            (std::min)(dllData.size(), size_t(120)));
    } else {
      preview = std::format("Header magic 0x{:04X} != 0x5A4D (MZ)", pDos->e_magic);
    }
    fail_with_error(ORION_ERROR_INVALID_MODULE_1, "Invalid DOS signature", preview);
    return false;
  }

  IMAGE_NT_HEADERS *pNt =
      reinterpret_cast<IMAGE_NT_HEADERS *>(dllData.data() + pDos->e_lfanew);
  if (pNt->Signature != IMAGE_NT_SIGNATURE) {
    fail_with_error(ORION_ERROR_INVALID_MODULE_2, "Invalid NT signature",
                    std::format("NT signature 0x{:08X} != 0x00004550 (PE00)", pNt->Signature));
    return false;
  }

  // We always inject the loader.dll bootstrap into our own launcher process.
  // The loaded Orion module handles finding/launching the actual game process.
  HANDLE hProc = GetCurrentProcess();
  bool bSuccess = false;

  if (hProc) {
    // 1. Map memory in target
    BYTE *pTargetBase = reinterpret_cast<BYTE *>(
        VirtualAllocEx(hProc, NULL, pNt->OptionalHeader.SizeOfImage,
                       MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (pTargetBase) {
      // 2. Map Sections
      WriteProcessMemory(hProc, pTargetBase, dllData.data(),
                         pNt->OptionalHeader.SizeOfHeaders, nullptr);
      IMAGE_SECTION_HEADER *pSec = IMAGE_FIRST_SECTION(pNt);
      for (UINT i = 0; i < pNt->FileHeader.NumberOfSections; ++i, ++pSec) {
        if (pSec->SizeOfRawData) {
          WriteProcessMemory(hProc, pTargetBase + pSec->VirtualAddress,
                             dllData.data() + pSec->PointerToRawData,
                             pSec->SizeOfRawData, nullptr);
        }
      }

      // 3. Setup Mapping Data & arguments
      ManualMappingData data = {};
      data.pLoadLibraryA = LoadLibraryA;
      data.pGetProcAddress = GetProcAddress;

      HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
      if (hNtDll) {
        data.pRtlAddFunctionTable = reinterpret_cast<BOOLEAN(WINAPI *)(
            PRUNTIME_FUNCTION, DWORD, DWORD64)>(
            GetProcAddress(hNtDll, "RtlAddFunctionTable"));
      }

      data.pBase = pTargetBase;
      strncpy_s(data.token, token.c_str(), sizeof(data.token) - 1);
      strncpy_s(data.app_id, app_id.c_str(), sizeof(data.app_id) - 1);
      data.pReportStage = report_loader_stage;
      data.pReportFinished = report_loader_finished;

      // 4. Inject Shellcode and mapping data
      BYTE *pMappingData = reinterpret_cast<BYTE *>(
          VirtualAllocEx(hProc, NULL, sizeof(ManualMappingData),
                         MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
      WriteProcessMemory(hProc, pMappingData, &data, sizeof(ManualMappingData),
                         nullptr);

      DWORD shellcodeSize = reinterpret_cast<DWORD_PTR>(LoaderShellcodeEnd) -
                            reinterpret_cast<DWORD_PTR>(LoaderShellcode);
      BYTE *pShellcode = reinterpret_cast<BYTE *>(
          VirtualAllocEx(hProc, NULL, shellcodeSize, MEM_COMMIT | MEM_RESERVE,
                         PAGE_EXECUTE_READWRITE));
      WriteProcessMemory(hProc, pShellcode, LoaderShellcode, shellcodeSize,
                         nullptr);

      // 5. Execute
      HANDLE hThread = CreateRemoteThread(
          hProc, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode),
          pMappingData, 0, NULL);
      if (hThread) {
        CloseHandle(hThread);
        bSuccess = true;
        set_stage("Bootstrap started...", 45);
      } else {
        DWORD winErr = GetLastError();
        fail_with_error(ORION_ERROR_CREATE_THREAD_1,
                        "CreateRemoteThread failed", "Win32 error: " + std::to_string(winErr));
      }
    } else {
      DWORD winErr = GetLastError();
      fail_with_error(ORION_ERROR_ALLOCATE_MEMORY_1,
                      "VirtualAllocEx failed in launcher", "Win32 error: " + std::to_string(winErr));
    }

    if (hProc != GetCurrentProcess())
      CloseHandle(hProc);
  } else {
    DWORD winErr = GetLastError();
    fail_with_error(ORION_ERROR_OPEN_PROCESS_TOKEN_1,
                    "OpenProcess failed for launcher PID", "Win32 error: " + std::to_string(winErr));
  }

  return bSuccess;
}
} // namespace loader
