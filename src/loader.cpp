#undef UNICODE
#undef _UNICODE
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <mutex>
#include "loader.hpp"
#include <http2client/http2client_easy.h>

#ifdef _DEBUG
#define LAUNCHER_BASE_URL "http://localhost:3000"
#else
#define LAUNCHER_BASE_URL "http://launcher.weave.su"
#endif

#define CDN_URL "http://cdn.weave.su"

#include <mutex>

namespace loader {

static std::mutex g_stage_mutex;
static std::string g_stage_name = "Ready";
static int g_stage_progress = 0;

void set_stage(const std::string &name, int progress) {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  g_stage_name = name;
  g_stage_progress = progress;
}

std::string get_stage_name() {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  return g_stage_name;
}

int get_stage_progress() {
  std::lock_guard<std::mutex> lock(g_stage_mutex);
  return g_stage_progress;
}

struct ManualMappingData {
  HMODULE(WINAPI *pLoadLibraryA)(LPCSTR);
  FARPROC(WINAPI *pGetProcAddress)(HMODULE, LPCSTR);
  BOOLEAN(WINAPI *pRtlAddFunctionTable)(PRUNTIME_FUNCTION, DWORD, DWORD64);

  BYTE *pBase;

  // Custom arguments to pass to DllMain
  char token[256];
  char app_id[64];
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
  set_stage("Connecting to CDN...", 10);
  http2client::EasyClient client(CDN_URL);
  client.Bearer(token).Timeout(15000);

  set_stage("Downloading payload...", 30);
  auto resp = client.Get("/loader.dll");
  if (!resp.ok() || resp.body.empty()) {
    set_stage("Download failed", 0);
    return false;
  }

  set_stage("Verifying binary headers...", 50);
  std::vector<BYTE> dllData(resp.body.begin(), resp.body.end());
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

  set_stage("Searching target process...", 65);

  bool bSuccess = false;
  set_stage("Opening target process...", 75);
  HANDLE hProc = GetCurrentProcess();
  if (hProc) {
    set_stage("Allocating memory in target...", 80);
    // 1. Map memory in target
    BYTE *pTargetBase = reinterpret_cast<BYTE *>(
        VirtualAllocEx(hProc, NULL, pNt->OptionalHeader.SizeOfImage,
                       MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (pTargetBase) {
      set_stage("Writing sections and headers...", 85);
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

      set_stage("Configuring mapping data...", 90);
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

      set_stage("Writing shellcode & mapping data...", 95);
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

      set_stage("Creating remote execution thread...", 98);
      // 5. Execute
      HANDLE hThread = CreateRemoteThread(
          hProc, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode),
          pMappingData, 0, NULL);
      if (hThread) {
        CloseHandle(hThread);
        bSuccess = true;
        set_stage("Payload injected successfully!", 100);
      } else {
        set_stage("CreateRemoteThread failed", 0);
      }
    } else {
      set_stage("VirtualAllocEx failed in target", 0);
    }

    CloseHandle(hProc);
  } else {
    set_stage("OpenProcess failed for target PID", 0);
  }

  return bSuccess;
}
} // namespace loader