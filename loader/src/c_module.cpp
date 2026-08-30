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

// ---------------------------------------------------------------------------
// Stage tracking
// ---------------------------------------------------------------------------

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
// CS2-specific constants
// ---------------------------------------------------------------------------

static constexpr const char *CS2_EXE        = "cs2.exe";
static constexpr const char *CS2_STEAM_URL  = "steam://rungameid/730";

// All of these must be present in the process module list before injection.
static const std::vector<const char *> CS2_REQUIRED_MODULES = {
  "engine2.dll",
  "client.dll",
  "schemasystem.dll",
  "tier0.dll",
  "vstdlib_s.dll",
  "inputsystem.dll",
  "navsystem.dll",
};

// ---------------------------------------------------------------------------
// Process helpers
// ---------------------------------------------------------------------------

static DWORD find_cs2_pid() {
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return 0;

  PROCESSENTRY32W pe;
  pe.dwSize = sizeof(pe);
  DWORD pid = 0;

  if (Process32FirstW(hSnap, &pe)) {
    do {
      if (_wcsicmp(pe.szExeFile, L"cs2.exe") == 0) {
        pid = pe.th32ProcessID;
        break;
      }
    } while (Process32NextW(hSnap, &pe));
  }

  CloseHandle(hSnap);
  return pid;
}

// ---------------------------------------------------------------------------
// Module snapshot & comparison
// ---------------------------------------------------------------------------

// Enumerate ALL DLL base-names loaded in the process into a lowercase vector.
// Single EnumProcessModules call per snapshot — avoids per-module overhead.
static std::vector<std::string> snapshot_modules(HANDLE hProc) {
  std::vector<std::string> result;
  HMODULE mods[2048];
  DWORD needed = 0;
  if (!EnumProcessModules(hProc, mods, sizeof(mods), &needed))
    return result;

  DWORD count = needed / sizeof(HMODULE);
  char baseName[MAX_PATH];
  result.reserve(count);
  for (DWORD i = 0; i < count; ++i) {
    if (GetModuleBaseNameA(hProc, mods[i], baseName, sizeof(baseName))) {
      std::string name = baseName;
      for (auto &c : name) c = (char)tolower((unsigned char)c);
      result.push_back(std::move(name));
    }
  }
  return result;
}

// Compare snapshot against required list.
// Returns the count of required entries found in the snapshot.
// Full match = return value equals required.size().
static size_t count_matched(const std::vector<std::string>   &snapshot,
                             const std::vector<const char *> &required) {
  size_t found = 0;
  for (const char *req : required) {
    std::string reqLow = req;
    for (auto &c : reqLow) c = (char)tolower((unsigned char)c);

    for (const auto &mod : snapshot) {
      if (mod == reqLow) {
        ++found;
        break;
      }
    }
  }
  return found;
}

// Poll every 500 ms until ALL required modules appear in the process snapshot.
// Updates stage label with count progress.  Returns false on timeout.
static bool wait_for_cs2_modules(HANDLE hProc, DWORD timeoutMs,
                                  int baseProgress, int maxProgress) {
  const size_t total     = CS2_REQUIRED_MODULES.size();
  const DWORD  pollMs    = 500;
  DWORD        elapsed   = 0;

  while (elapsed < timeoutMs) {
    auto   snap   = snapshot_modules(hProc);
    size_t loaded = count_matched(snap, CS2_REQUIRED_MODULES);

    int pct = baseProgress +
              (int)((float)loaded / (float)total * (maxProgress - baseProgress));

    char buf[256];
    sprintf_s(buf, sizeof(buf),
              "Waiting for CS2 modules... (%zu / %zu loaded)", loaded, total);
    c_module::set_stage(buf, pct);

    if (loaded == total)
      return true;

    Sleep(pollMs);
    elapsed += pollMs;
  }
  return false;
}

// ---------------------------------------------------------------------------
// Shellcode (executes inside cs2.exe via CreateRemoteThread)
// ---------------------------------------------------------------------------

#pragma runtime_checks("", off)
#pragma optimize("", off)
void __stdcall LoaderShellcode(ManualMappingData *pData) {
  if (!pData) return;

  BYTE *pBase = pData->pBase;
  auto *pOpt  = &reinterpret_cast<IMAGE_NT_HEADERS *>(
                      pBase + reinterpret_cast<IMAGE_DOS_HEADER *>(pBase)->e_lfanew)
                      ->OptionalHeader;

  auto _LoadLibraryA   = pData->pLoadLibraryA;
  auto _GetProcAddress = pData->pGetProcAddress;

  // Resolve imports
  if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
    auto *pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(
        pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    while (pImportDescr->Name) {
      char     *szMod   = reinterpret_cast<char *>(pBase + pImportDescr->Name);
      HINSTANCE hDll    = _LoadLibraryA(szMod);
      ULONG_PTR *pThunk = reinterpret_cast<ULONG_PTR *>(pBase + pImportDescr->OriginalFirstThunk);
      ULONG_PTR *pFunc  = reinterpret_cast<ULONG_PTR *>(pBase + pImportDescr->FirstThunk);
      if (!pThunk) pThunk = pFunc;
      for (; *pThunk; ++pThunk, ++pFunc) {
        if (IMAGE_SNAP_BY_ORDINAL(*pThunk)) {
          *pFunc = (ULONG_PTR)_GetProcAddress(hDll, reinterpret_cast<char *>(*pThunk & 0xFFFF));
        } else {
          auto *pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(pBase + *pThunk);
          *pFunc = (ULONG_PTR)_GetProcAddress(hDll, pImport->Name);
        }
      }
      ++pImportDescr;
    }
  }

  // Fix relocations
  if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
    auto *pReloc = reinterpret_cast<IMAGE_BASE_RELOCATION *>(
        pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    ULONG_PTR delta = reinterpret_cast<ULONG_PTR>(pBase) - pOpt->ImageBase;
    while (pReloc->VirtualAddress) {
      if (pReloc->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION)) {
        size_t count = (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
        WORD  *list  = reinterpret_cast<WORD *>(pReloc + 1);
        for (size_t i = 0; i < count; ++i) {
          if (list[i]) {
            WORD type   = list[i] >> 12;
            WORD offset = list[i] & 0xFFF;
            if (type == IMAGE_REL_BASED_HIGHLOW || type == IMAGE_REL_BASED_DIR64) {
              *reinterpret_cast<ULONG_PTR *>(pBase + pReloc->VirtualAddress + offset) += delta;
            }
          }
        }
      }
      pReloc = reinterpret_cast<IMAGE_BASE_RELOCATION *>(
          reinterpret_cast<BYTE *>(pReloc) + pReloc->SizeOfBlock);
    }
  }

  // SEH (64-bit exception table)
  if (pData->pRtlAddFunctionTable &&
      pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size) {
    auto *pFuncTable = reinterpret_cast<PRUNTIME_FUNCTION>(
        pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress);
    DWORD entryCount = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size /
                       sizeof(RUNTIME_FUNCTION);
    pData->pRtlAddFunctionTable(pFuncTable, entryCount, (DWORD64)pBase);
  }

  // TLS callbacks
  if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
    auto *pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY *>(
        pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
    auto *pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK *>(pTLS->AddressOfCallBacks);
    while (pCallback && *pCallback) {
      (*pCallback)(pBase, DLL_PROCESS_ATTACH, nullptr);
      ++pCallback;
    }
  }

  // Entry point
  if (pOpt->AddressOfEntryPoint) {
    using DllEntry_t = BOOL(WINAPI *)(void *, DWORD, void *);
    reinterpret_cast<DllEntry_t>(pBase + pOpt->AddressOfEntryPoint)(
        pBase, DLL_PROCESS_ATTACH, pData);
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
  if (!pData) return false;
  return verify_auth(pData->token, pData->app_id);
}

bool c_module::verify_auth(const std::string &token,
                           const std::string &app_id) {
  using namespace secure_proto;
  set_finished(false);

  // ── 1. Check if CS2 is running; launch via Steam if not ──────────────────
  set_stage("Checking if CS2 is running...", 5);
  Sleep(300);

  DWORD pid = find_cs2_pid();
  if (!pid) {
    set_stage("Launching CS2 via Steam...", 8);
    ShellExecuteA(NULL, "open", CS2_STEAM_URL, NULL, NULL, SW_SHOWNORMAL);
    Sleep(2000);

    // Wait up to 90 s for the process to appear
    const DWORD launchTimeout = 90000;
    DWORD waited = 0;
    while (waited < launchTimeout) {
      pid = find_cs2_pid();
      if (pid) break;
      Sleep(1000);
      waited += 1000;
      char buf[128];
      sprintf_s(buf, sizeof(buf),
                "Waiting for CS2 to start... (%u s)", waited / 1000);
      set_stage(buf, 10);
    }

    if (!pid) {
      set_stage("CS2 did not start within 90 seconds", 0);
      return false;
    }
  }

  // ── 2. Wait for all required CS2 modules ─────────────────────────────────
  set_stage("CS2 process found, checking modules...", 12);
  Sleep(300);

  HANDLE hProcCheck = OpenProcess(
      PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if (!hProcCheck) {
    set_stage("Cannot open CS2 process for module check", 0);
    return false;
  }

  // Poll snapshot vs required list until all match or 2-min timeout
  bool modulesReady = wait_for_cs2_modules(hProcCheck, 120000, 12, 20);
  CloseHandle(hProcCheck);

  if (!modulesReady) {
    set_stage("Timed out waiting for CS2 modules", 0);
    return false;
  }

  // Extra settle: some subsystems init after their DLL appears
  set_stage("All CS2 modules loaded, waiting for engine init...", 20);
  Sleep(3000);

  // ── 3. Authenticate ───────────────────────────────────────────────────────
  set_stage("Connecting to SQP auth server...", 28);
  Sleep(300);

  SecureClient client("api.weave.su", 9055,
      "07a37cbc142093c8b755dc1b10e86cb426374ad16aa853ed0bdfc0b2b86d1c7c");

  NetResult res = client.connect(5000);
  if (!res.ok()) {
    set_stage("Failed to connect to auth server", 0);
    return false;
  }

  set_stage("Verifying session...", 38);
  Sleep(300);

  AuthVerifyRequest verify_req;
  verify_req.launcher_token = token;
  verify_req.product_id     = app_id;

  Response verify_raw;
  res = client.post_binary("/api/v1/auth", verify_req, &verify_raw);
  if (!res.ok() || verify_raw.status_code != 200) {
    set_stage("Session verification failed", 0);
    return false;
  }

  if (!SecureClient::unpack_binary_response(verify_raw, m_auth)) {
    set_stage("Failed to unpack auth response", 0);
    return false;
  }

  // ── 4. Download payload ───────────────────────────────────────────────────
  set_stage("Downloading payload from S3...", 52);
  Sleep(300);

  InjectDownloadReq dl_req;
  dl_req.launcher_token = token;
  dl_req.product_id     = app_id;

  Response dl_raw;
  res = client.post_binary("/api/v1/download", dl_req, &dl_raw);
  if (!res.ok() || dl_raw.status_code != 200) {
    set_stage("Download request failed", 0);
    return false;
  }

  InjectDownloadResp dl_ack;
  if (!SecureClient::unpack_binary_response(dl_raw, dl_ack)) {
    set_stage("Failed to unpack payload bytes", 0);
    return false;
  }

  if (dl_ack.status != 200 || dl_ack.data.empty()) {
    set_stage("Invalid payload response from server", 0);
    return false;
  }

  m_payload_data = std::move(dl_ack.data);

  // ── 5. Verify PE ──────────────────────────────────────────────────────────
  set_stage("Verifying PE headers...", 68);
  Sleep(200);

  std::vector<BYTE> dllData(m_payload_data.begin(), m_payload_data.end());
  if (dllData.size() < sizeof(IMAGE_DOS_HEADER)) {
    set_stage("Invalid PE: file too small", 0);
    return false;
  }

  auto *pDos = reinterpret_cast<IMAGE_DOS_HEADER *>(dllData.data());
  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) {
    set_stage("Invalid PE: bad DOS signature", 0);
    return false;
  }

  auto *pNt = reinterpret_cast<IMAGE_NT_HEADERS *>(dllData.data() + pDos->e_lfanew);
  if (pNt->Signature != IMAGE_NT_SIGNATURE) {
    set_stage("Invalid PE: bad NT signature", 0);
    return false;
  }

  // ── 6. Inject into CS2 ───────────────────────────────────────────────────
  set_stage("Opening CS2 process for injection...", 76);
  Sleep(200);

  // Re-query PID (process might have restarted)
  pid = find_cs2_pid();
  if (!pid) {
    set_stage("CS2 process gone before injection", 0);
    return false;
  }

  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (!hProc) {
    set_stage("OpenProcess failed (run as Administrator)", 0);
    return false;
  }

  bool bSuccess = false;

  set_stage("Allocating memory in CS2...", 83);
  Sleep(200);

  BYTE *pTargetBase = reinterpret_cast<BYTE *>(
      VirtualAllocEx(hProc, NULL, pNt->OptionalHeader.SizeOfImage,
                     MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
  if (!pTargetBase) {
    set_stage("VirtualAllocEx failed", 0);
    CloseHandle(hProc);
    return false;
  }

  set_stage("Writing PE sections to CS2...", 88);
  Sleep(200);

  WriteProcessMemory(hProc, pTargetBase, dllData.data(),
                     pNt->OptionalHeader.SizeOfHeaders, nullptr);

  auto *pSec = IMAGE_FIRST_SECTION(pNt);
  for (UINT i = 0; i < pNt->FileHeader.NumberOfSections; ++i, ++pSec) {
    if (pSec->SizeOfRawData) {
      WriteProcessMemory(hProc, pTargetBase + pSec->VirtualAddress,
                         dllData.data() + pSec->PointerToRawData,
                         pSec->SizeOfRawData, nullptr);
    }
  }

  set_stage("Configuring mapping data...", 93);
  Sleep(200);

  ManualMappingData data = {};
  data.pLoadLibraryA   = LoadLibraryA;
  data.pGetProcAddress = GetProcAddress;
  data.pBase           = pTargetBase;

  HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
  if (hNtDll)
    data.pRtlAddFunctionTable = reinterpret_cast<BOOLEAN(WINAPI *)(
        PRUNTIME_FUNCTION, DWORD, DWORD64)>(
        GetProcAddress(hNtDll, "RtlAddFunctionTable"));

  strncpy_s(data.token,  token.c_str(),  sizeof(data.token) - 1);
  strncpy_s(data.app_id, app_id.c_str(), sizeof(data.app_id) - 1);

  set_stage("Launching remote thread in CS2...", 97);
  Sleep(200);

  BYTE *pMappingData = reinterpret_cast<BYTE *>(
      VirtualAllocEx(hProc, NULL, sizeof(ManualMappingData),
                     MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
  WriteProcessMemory(hProc, pMappingData, &data, sizeof(ManualMappingData), nullptr);

  DWORD scSize    = reinterpret_cast<DWORD_PTR>(LoaderShellcodeEnd) -
                    reinterpret_cast<DWORD_PTR>(LoaderShellcode);
  BYTE *pShellcode = reinterpret_cast<BYTE *>(
      VirtualAllocEx(hProc, NULL, scSize, MEM_COMMIT | MEM_RESERVE,
                     PAGE_EXECUTE_READWRITE));
  WriteProcessMemory(hProc, pShellcode, LoaderShellcode, scSize, nullptr);

  HANDLE hThread = CreateRemoteThread(
      hProc, NULL, 0,
      reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode),
      pMappingData, 0, NULL);

  if (hThread) {
    CloseHandle(hThread);
    bSuccess = true;
    set_stage("Payload injected into CS2!", 100);
    set_finished(true);
  } else {
    set_stage("CreateRemoteThread failed", 0);
  }

  CloseHandle(hProc);
  return bSuccess;
}
