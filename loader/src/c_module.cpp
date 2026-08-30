#include "c_module.hpp"
#include <secure_client.hpp>
#include <stdio.h>
#include <winuser.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <shellapi.h>
#include <mutex>
#include <string>
#include <vector>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shell32.lib")

static std::mutex g_module_stage_mutex;
static std::string g_module_stage_name = "Ready";
static int g_module_stage_progress = 0;
static bool g_module_is_finished = false;

void c_module::set_stage(const std::string &name, int progress) {
  std::lock_guard<std::mutex> lock(g_module_stage_mutex);
  g_module_stage_name = name;
  g_module_stage_progress = progress;
}

void c_module::set_finished(bool finished) {
  std::lock_guard<std::mutex> lock(g_module_stage_mutex);
  g_module_is_finished = finished;
}

std::string c_module::get_stage_name() {
  std::lock_guard<std::mutex> lock(g_module_stage_mutex);
  return g_module_stage_name;
}

int c_module::get_stage_progress() {
  std::lock_guard<std::mutex> lock(g_module_stage_mutex);
  return g_module_stage_progress;
}

bool c_module::is_finished() {
  std::lock_guard<std::mutex> lock(g_module_stage_mutex);
  return g_module_is_finished;
}

// ---------------------------------------------------------------------------
// Process helpers
// ---------------------------------------------------------------------------

static DWORD find_pid_by_name(const char *name) {
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return 0;
  PROCESSENTRY32W pe;
  pe.dwSize = sizeof(pe);
  DWORD pid = 0;
  wchar_t wname[MAX_PATH];
  MultiByteToWideChar(CP_ACP, 0, name, -1, wname, MAX_PATH);
  if (Process32FirstW(hSnap, &pe)) {
    do {
      if (_wcsicmp(pe.szExeFile, wname) == 0) {
        pid = pe.th32ProcessID;
        break;
      }
    } while (Process32NextW(hSnap, &pe));
  }
  CloseHandle(hSnap);
  return pid;
}

// Check if a specific module (DLL name, case-insensitive) is loaded in the
// given process.
static bool is_module_loaded(HANDLE hProc, const char *modName) {
  HMODULE mods[1024];
  DWORD needed = 0;
  if (!EnumProcessModules(hProc, mods, sizeof(mods), &needed))
    return false;

  DWORD count = needed / sizeof(HMODULE);
  char baseName[MAX_PATH];
  for (DWORD i = 0; i < count; ++i) {
    if (GetModuleBaseNameA(hProc, mods[i], baseName, sizeof(baseName))) {
      if (_stricmp(baseName, modName) == 0)
        return true;
    }
  }
  return false;
}

// Wait until all required modules are loaded or timeout (ms) is exceeded.
// Returns true if all modules loaded within time.
static bool wait_for_modules(HANDLE hProc,
                              const std::vector<const char *> &modules,
                              DWORD timeoutMs,
                              const char *stageFmt,
                              int baseProgress,
                              int maxProgress) {
  DWORD elapsed = 0;
  const DWORD pollMs = 500;
  size_t total = modules.size();

  while (elapsed < timeoutMs) {
    size_t loaded = 0;
    for (auto *mod : modules) {
      if (is_module_loaded(hProc, mod))
        ++loaded;
    }

    // Calculate progress within the given range
    int pct = baseProgress + (int)((float)loaded / total * (maxProgress - baseProgress));
    char buf[256];
    sprintf_s(buf, sizeof(buf), stageFmt, (int)loaded, (int)total);
    c_module::set_stage(buf, pct);

    if (loaded == total)
      return true;

    Sleep(pollMs);
    elapsed += pollMs;
  }
  return false;
}

// ---------------------------------------------------------------------------
// Shellcode (runs inside target process)
// ---------------------------------------------------------------------------

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

  // Call Entry Point
  if (pOpt->AddressOfEntryPoint) {
    using DllEntry_t = BOOL(WINAPI *)(void *, DWORD, void *);
    auto DllEntry =
        reinterpret_cast<DllEntry_t>(pBase + pOpt->AddressOfEntryPoint);
    DllEntry(pBase, DLL_PROCESS_ATTACH, pData);
  }
}
void __stdcall LoaderShellcodeEnd() {}
#pragma optimize("", on)
#pragma runtime_checks("", restore)

// ---------------------------------------------------------------------------
// c_module
// ---------------------------------------------------------------------------

c_module &c_module::instance() {
  static c_module inst;
  return inst;
}

bool c_module::init(ManualMappingData *pData) {
  m_data = pData;
  if (!pData)
    return false;

  std::string token  = pData->token;
  std::string app_id = pData->app_id;

  return verify_auth(token, app_id);
}

bool c_module::verify_auth(const std::string &token,
                           const std::string &app_id) {
  using namespace secure_proto;

  set_finished(false);

  // ------------------------------------------------------------------
  // Determine target process name and Steam AppID
  // ------------------------------------------------------------------
  const char *targetExe = "cs2.exe";
  const char *steamUrl  = "steam://rungameid/730";

  if (app_id == "rust") {
    targetExe = "RustClient.exe";
    steamUrl  = "steam://rungameid/252490";
  }

  // Required modules that must be present before injection is safe.
  // These are the core CS2 engine modules that load during startup.
  std::vector<const char *> requiredModules;
  if (_stricmp(targetExe, "cs2.exe") == 0) {
    requiredModules = {
      "engine2.dll",
      "client.dll",
      "schemasystem.dll",
      "tier0.dll",
      "vstdlib_s.dll",
    };
  } else if (_stricmp(targetExe, "RustClient.exe") == 0) {
    requiredModules = {
      "GameAssembly.dll",
      "UnityPlayer.dll",
    };
  }

  // ------------------------------------------------------------------
  // Step 1 – Check / launch game
  // ------------------------------------------------------------------
  set_stage("Checking if game is running...", 5);
  Sleep(300);

  DWORD pid = find_pid_by_name(targetExe);
  if (!pid) {
    set_stage("Launching game via Steam...", 8);
    ShellExecuteA(NULL, "open", steamUrl, NULL, NULL, SW_SHOWNORMAL);
    Sleep(2000); // give Steam a moment to start the launch sequence

    // Wait up to 90 seconds for the process to appear
    set_stage("Waiting for game process to start...", 10);
    const DWORD launchTimeout = 90000;
    DWORD waited = 0;
    while (waited < launchTimeout) {
      pid = find_pid_by_name(targetExe);
      if (pid) break;
      Sleep(1000);
      waited += 1000;

      // Update dots animation so the UI doesn't look frozen
      char buf[128];
      sprintf_s(buf, sizeof(buf), "Waiting for %s... (%us)", targetExe, waited / 1000);
      set_stage(buf, 10);
    }

    if (!pid) {
      set_stage("Game process did not start within 90 seconds", 0);
      return false;
    }
  }

  set_stage("Game process found, waiting for modules...", 12);
  Sleep(300);

  // ------------------------------------------------------------------
  // Step 2 – Wait for required modules to load (real polling)
  // ------------------------------------------------------------------
  if (!requiredModules.empty()) {
    HANDLE hProcCheck = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcCheck) {
      set_stage("Cannot open game process for module check", 0);
      return false;
    }

    bool modulesReady = wait_for_modules(
        hProcCheck,
        requiredModules,
        120000,   // up to 2 minutes for all modules to load
        "Loading game modules... (%d/%d ready)",
        12,       // start progress %
        20        // end progress % (leaves room for auth steps below)
    );

    CloseHandle(hProcCheck);

    if (!modulesReady) {
      set_stage("Timed out waiting for game modules to load", 0);
      return false;
    }

    // Small extra settle time after all modules report as loaded –
    // some game subsystems initialise after the DLL appears.
    set_stage("Game modules ready, waiting for engine init...", 20);
    Sleep(3000);
  }

  // ------------------------------------------------------------------
  // Step 3 – Authenticate
  // ------------------------------------------------------------------
  set_stage("Connecting to secure SQP auth server...", 25);
  Sleep(300);

  std::string host = "api.weave.su";
  uint16_t port = 9055;
  std::string pubkey_hex =
      "07a37cbc142093c8b755dc1b10e86cb426374ad16aa853ed0bdfc0b2b86d1c7c";

  SecureClient client(host, port, pubkey_hex);

  NetResult res = client.connect(5000);
  if (!res.ok()) {
    set_stage("Failed to connect to auth server", 0);
    return false;
  }

  set_stage("Verifying session authentication...", 40);
  Sleep(300);

  AuthVerifyRequest verify_req;
  verify_req.launcher_token = token;
  verify_req.product_id     = app_id;

  Response verify_raw_response;
  res = client.post_binary("/api/v1/auth", verify_req, &verify_raw_response);
  if (!res.ok() || verify_raw_response.status_code != 200) {
    set_stage("Session verification failed", 0);
    return false;
  }

  if (!SecureClient::unpack_binary_response(verify_raw_response, m_auth)) {
    set_stage("Failed to unpack auth response", 0);
    return false;
  }

  // ------------------------------------------------------------------
  // Step 4 – Download payload
  // ------------------------------------------------------------------
  set_stage("Downloading game payload from S3...", 55);
  Sleep(300);

  InjectDownloadReq download_req;
  download_req.launcher_token = token;
  download_req.product_id     = app_id;

  Response download_raw_response;
  res = client.post_binary("/api/v1/download", download_req,
                           &download_raw_response);
  if (!res.ok() || download_raw_response.status_code != 200) {
    set_stage("Download payload request failed", 0);
    return false;
  }

  InjectDownloadResp download_ack;
  if (!SecureClient::unpack_binary_response(download_raw_response,
                                            download_ack)) {
    set_stage("Failed to unpack payload bytes", 0);
    return false;
  }

  if (download_ack.status != 200 || download_ack.data.empty()) {
    set_stage("Invalid download status or empty payload", 0);
    return false;
  }

  m_payload_data = std::move(download_ack.data);

  // ------------------------------------------------------------------
  // Step 5 – Verify PE and inject
  // ------------------------------------------------------------------
  set_stage("Verifying PE binary headers...", 70);
  Sleep(200);

  std::vector<BYTE> dllData(m_payload_data.begin(), m_payload_data.end());
  if (dllData.size() < sizeof(IMAGE_DOS_HEADER)) {
    set_stage("Invalid PE file size", 0);
    return false;
  }

  IMAGE_DOS_HEADER *pDos = reinterpret_cast<IMAGE_DOS_HEADER *>(dllData.data());
  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) {
    set_stage("Invalid DOS signature", 0);
    return false;
  }

  IMAGE_NT_HEADERS *pNt =
      reinterpret_cast<IMAGE_NT_HEADERS *>(dllData.data() + pDos->e_lfanew);
  if (pNt->Signature != IMAGE_NT_SIGNATURE) {
    set_stage("Invalid NT signature", 0);
    return false;
  }

  set_stage("Opening target process for injection...", 78);
  Sleep(200);

  // Re-query PID in case it changed (unlikely but safe)
  pid = find_pid_by_name(targetExe);
  if (!pid) {
    set_stage("Cannot find game process for injection", 0);
    return false;
  }

  bool bSuccess = false;
  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (!hProc) {
    set_stage("OpenProcess failed (try running as Administrator)", 0);
    return false;
  }

  set_stage("Allocating virtual memory in target...", 84);
  Sleep(200);

  BYTE *pTargetBase = reinterpret_cast<BYTE *>(
      VirtualAllocEx(hProc, NULL, pNt->OptionalHeader.SizeOfImage,
                     MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
  if (pTargetBase) {
    set_stage("Writing PE sections and headers...", 89);
    Sleep(200);

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

    set_stage("Configuring manual mapping & imports...", 93);
    Sleep(200);

    ManualMappingData data = {};
    data.pLoadLibraryA  = LoadLibraryA;
    data.pGetProcAddress = GetProcAddress;

    HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
    if (hNtDll) {
      data.pRtlAddFunctionTable = reinterpret_cast<BOOLEAN(WINAPI *)(
          PRUNTIME_FUNCTION, DWORD, DWORD64)>(
          GetProcAddress(hNtDll, "RtlAddFunctionTable"));
    }

    data.pBase = pTargetBase;
    strncpy_s(data.token,  token.c_str(),  sizeof(data.token) - 1);
    strncpy_s(data.app_id, app_id.c_str(), sizeof(data.app_id) - 1);

    set_stage("Executing remote payload thread...", 97);
    Sleep(200);

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

    HANDLE hThread = CreateRemoteThread(
        hProc, NULL, 0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode),
        pMappingData, 0, NULL);
    if (hThread) {
      CloseHandle(hThread);
      bSuccess = true;
      set_stage("Payload injected successfully!", 100);
      set_finished(true);
    } else {
      set_stage("CreateRemoteThread failed", 0);
    }
  } else {
    set_stage("VirtualAllocEx failed in target", 0);
  }

  CloseHandle(hProc);
  return bSuccess;
}
