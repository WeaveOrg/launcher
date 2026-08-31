#undef UNICODE
#undef _UNICODE
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include "loader.hpp"
#include <http2client/http2client_easy.h>

#ifdef _DEBUG
#define LAUNCHER_BASE_URL "http://localhost:3000"
#else
#define LAUNCHER_BASE_URL "http://launcher.weave.su"
#endif

#define CDN_URL "http://cdn.weave.su"

namespace loader {

static std::mutex g_stage_mutex;
static std::string g_stage_name = "Ready";
static int g_stage_progress = 0;
static bool g_is_finished = false;
static bool g_was_successful = false;

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

static void WINAPI report_loader_stage(const char *name, int progress) {
  // Bootstrap occupies 0-45%; loader.dll owns the remaining 55%.
  progress = (std::max)(0, (std::min)(100, progress));
  set_stage(name ? name : "Working...", 45 + progress * 55 / 100);
}

static void WINAPI report_loader_finished(BOOL success) {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  g_was_successful = success != FALSE;
  g_is_finished = true;
  if (g_was_successful)
    g_stage_progress = 100;
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
  }
  set_stage("Connecting to CDN...", 5);
  Sleep(400);

  http2client::EasyClient client(CDN_URL);
  client.Bearer(token).Timeout(15000);

  set_stage("Downloading loader.dll...", 12);
  Sleep(450);

  auto resp = client.Get("/loader.dll");
  if (!resp.ok() || resp.body.empty()) {
    set_stage("Download failed", 0);
    report_loader_finished(FALSE);
    return false;
  }

  set_stage("Loader.dll received (" + std::to_string(resp.body.size()) + " bytes)", 25);
  Sleep(350);

  std::vector<BYTE> dllData(resp.body.begin(), resp.body.end());
  if (dllData.size() < sizeof(IMAGE_DOS_HEADER)) {
    set_stage("Invalid PE file size", 0);
    report_loader_finished(FALSE);
    return false;
  }

  IMAGE_DOS_HEADER *pDos = reinterpret_cast<IMAGE_DOS_HEADER *>(dllData.data());
  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) {
    set_stage("Invalid DOS signature", 0);
    report_loader_finished(FALSE);
    return false;
  }

  IMAGE_NT_HEADERS *pNt =
      reinterpret_cast<IMAGE_NT_HEADERS *>(dllData.data() + pDos->e_lfanew);
  if (pNt->Signature != IMAGE_NT_SIGNATURE) {
    set_stage("Invalid NT signature", 0);
    report_loader_finished(FALSE);
    return false;
  }

  set_stage("PE headers verified", 30);
  Sleep(300);

  // --- Phase 2: Manual Mapping ---
  set_stage("Loading bootstrap module...", 33);
  Sleep(350);

  std::string targetProc = app_id + ".exe";
  if (app_id == "cs2" || app_id == "6a943ac671805d202d5fc1e0")
    targetProc = "cs2.exe";
  else if (app_id == "rust")
    targetProc = "RustClient.exe";

  DWORD pid = FindTargetPid(targetProc);
  if (pid == 0)
    pid = GetCurrentProcessId();

  bool bSuccess = false;
  set_stage("Opening launcher process...", 35);
  Sleep(300);

  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (!hProc)
    hProc = GetCurrentProcess();

  if (hProc) {
    set_stage("Allocating bootstrap memory...", 37);
    Sleep(300);

    // 1. Map memory in target
    BYTE *pTargetBase = reinterpret_cast<BYTE *>(
        VirtualAllocEx(hProc, NULL, pNt->OptionalHeader.SizeOfImage,
                       MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (pTargetBase) {
      set_stage("Writing bootstrap module...", 39);
      Sleep(250);

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

      set_stage("Configuring bootstrap module...", 41);
      Sleep(250);

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

      set_stage("Starting bootstrap module...", 44);
      Sleep(250);

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
        set_stage("CreateRemoteThread failed", 0);
        report_loader_finished(FALSE);
      }
    } else {
      set_stage("VirtualAllocEx failed in launcher", 0);
      report_loader_finished(FALSE);
    }

    if (hProc != GetCurrentProcess())
      CloseHandle(hProc);
  } else {
    set_stage("OpenProcess failed for launcher PID", 0);
    report_loader_finished(FALSE);
  }

  return bSuccess;
}
} // namespace loader
