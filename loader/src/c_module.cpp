#include "c_module.hpp"
#include <secure_client.hpp>
#include <stdio.h>
#include <winuser.h>
#include <tlhelp32.h>

static DWORD find_pid_by_name(const char *name) {
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return 0;
  PROCESSENTRY32W pe;
  pe.dwSize = sizeof(pe);
  DWORD pid = 0;
  // Convert narrow name to wide for comparison
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

bool c_module::verify_auth(const std::string &token,
                           const std::string &app_id) {
  using namespace secure_proto;

  std::string host = "api.weave.su";
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
    sprintf_s(err, sizeof(err), "Verify Request Failed! Error: %s (Status: %d)",
              res.message(), verify_raw_response.status_code);
    MessageBoxA(NULL, err, "Weave Module Auth Failed", MB_OK | MB_ICONERROR);
    return false;
  }

  if (!SecureClient::unpack_binary_response(verify_raw_response, m_auth)) {
    MessageBoxA(NULL, "Failed to unpack auth response payload!",
                "Weave Module Error", MB_OK | MB_ICONERROR);
    return false;
  }

  MessageBoxA(NULL, m_auth.username.c_str(), "Weave Module Auth",
              MB_OK | MB_ICONINFORMATION);

  // Download payload
  InjectDownloadReq download_req;
  download_req.launcher_token = token;
  download_req.product_id = app_id;

  Response download_raw_response;
  res = client.post_binary("/api/v1/download", download_req,
                           &download_raw_response);
  if (!res.ok() || download_raw_response.status_code != 200) {
    char err[512];
    sprintf_s(err, sizeof(err),
              "Download Request Failed! Error: %s (Status: %d)", res.message(),
              download_raw_response.status_code);
    MessageBoxA(NULL, err, "Weave Module Download Failed",
                MB_OK | MB_ICONERROR);
    return false;
  }

  InjectDownloadResp download_ack;
  if (!SecureClient::unpack_binary_response(download_raw_response,
                                            download_ack)) {
    MessageBoxA(NULL, "Failed to unpack download response payload!",
                "Weave Module Error", MB_OK | MB_ICONERROR);
    return false;
  }

  if (download_ack.status != 200 || download_ack.data.empty()) {
    char err[512];
    sprintf_s(err, sizeof(err), "Download status: %u, data size: %zu bytes",
              download_ack.status, download_ack.data.size());
    MessageBoxA(NULL, err, "Weave Module Download Error", MB_OK | MB_ICONERROR);
    return false;
  }

  m_payload_data = std::move(download_ack.data);

  char msg[256];
  sprintf_s(msg, sizeof(msg),
            "Successfully downloaded payload DLL (%zu bytes)!",
            m_payload_data.size());
  MessageBoxA(NULL, msg, "Weave Module Download", MB_OK | MB_ICONINFORMATION);

  std::vector<BYTE> dllData(m_payload_data.begin(), m_payload_data.end());
  if (dllData.size() < sizeof(IMAGE_DOS_HEADER)) {
    return false;
  }

  IMAGE_DOS_HEADER *pDos = reinterpret_cast<IMAGE_DOS_HEADER *>(dllData.data());
  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) {
    return false;
  }

  IMAGE_NT_HEADERS *pNt =
      reinterpret_cast<IMAGE_NT_HEADERS *>(dllData.data() + pDos->e_lfanew);
  if (pNt->Signature != IMAGE_NT_SIGNATURE) {
    return false;
  }

  bool bSuccess = false;
  DWORD pid = find_pid_by_name("cs2.exe");
  if (!pid) {
    return false;
  }

  HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (!hProc) {
    return false;
  }

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
    }
  }

  CloseHandle(hProc);

  return true;
}
