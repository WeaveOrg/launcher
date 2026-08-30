#undef UNICODE
#undef _UNICODE
#include "loader.hpp"
#include <windows.h>
#include <string>
#include <vector>
#include <tlhelp32.h>

namespace loader {

    struct ManualMappingData {
        HMODULE(WINAPI* pLoadLibraryA)(LPCSTR);
        FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);
        BOOLEAN(WINAPI* pRtlAddFunctionTable)(PRUNTIME_FUNCTION, DWORD, DWORD64);
        
        BYTE* pBase;
        
        // Custom arguments to pass to DllMain
        char token[256];
        char app_id[64];
    };

    // The shellcode that will execute in the target process
#pragma runtime_checks( "", off )
#pragma optimize( "", off )
    void __stdcall LoaderShellcode(ManualMappingData* pData) {
        if (!pData) return;

        BYTE* pBase = pData->pBase;
        auto* pOpt = &reinterpret_cast<IMAGE_NT_HEADERS*>(pBase + reinterpret_cast<IMAGE_DOS_HEADER*>(pBase)->e_lfanew)->OptionalHeader;

        auto _LoadLibraryA = pData->pLoadLibraryA;
        auto _GetProcAddress = pData->pGetProcAddress;

        // Resolve Imports
        if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
            auto* pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
            while (pImportDescr->Name) {
                char* szMod = reinterpret_cast<char*>(pBase + pImportDescr->Name);
                HINSTANCE hDll = _LoadLibraryA(szMod);

                ULONG_PTR* pThunkRef = reinterpret_cast<ULONG_PTR*>(pBase + pImportDescr->OriginalFirstThunk);
                ULONG_PTR* pFuncRef = reinterpret_cast<ULONG_PTR*>(pBase + pImportDescr->FirstThunk);

                if (!pThunkRef) pThunkRef = pFuncRef;

                for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
                    if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
                        *pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, reinterpret_cast<char*>(*pThunkRef & 0xFFFF));
                    } else {
                        auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(pBase + (*pThunkRef));
                        *pFuncRef = (ULONG_PTR)_GetProcAddress(hDll, pImport->Name);
                    }
                }
                ++pImportDescr;
            }
        }

        // Fix Relocations
        if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
            auto* pReloc = reinterpret_cast<IMAGE_BASE_RELOCATION*>(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
            BYTE* pBaseDelta = pBase - pOpt->ImageBase;
            while (pReloc->VirtualAddress) {
                if (pReloc->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION)) {
                    int count = (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                    WORD* list = reinterpret_cast<WORD*>(pReloc + 1);
                    for (int i = 0; i < count; ++i) {
                        if (list[i] >> 12 == IMAGE_REL_BASED_DIR64) {
                            *reinterpret_cast<ULONG_PTR*>(pBase + pReloc->VirtualAddress + (list[i] & 0xFFF)) += (ULONG_PTR)pBaseDelta;
                        }
                    }
                }
                pReloc = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(pReloc) + pReloc->SizeOfBlock);
            }
        }

        // Register Exception Handlers (SEH)
        if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size) {
            auto pExceptionDir = reinterpret_cast<PRUNTIME_FUNCTION>(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress);
            DWORD dwExceptionCount = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size / sizeof(RUNTIME_FUNCTION);
            if (pData->pRtlAddFunctionTable) {
                pData->pRtlAddFunctionTable(pExceptionDir, dwExceptionCount, reinterpret_cast<DWORD64>(pBase));
            }
        }

        // Execute TLS Callbacks
        if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
            auto* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
            auto* pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
            for (; pCallback && *pCallback; ++pCallback) {
                (*pCallback)(pBase, DLL_PROCESS_ATTACH, pData);
            }
        }

        // Call DllMain with our custom struct passed into lpvReserved
        if (pOpt->AddressOfEntryPoint) {
            auto DllMain = reinterpret_cast<BOOL(WINAPI*)(HMODULE, DWORD, LPVOID)>(pBase + pOpt->AddressOfEntryPoint);
            DllMain(reinterpret_cast<HMODULE>(pBase), DLL_PROCESS_ATTACH, pData);
        }
    }
    // Stub to calculate shellcode size
    void __stdcall LoaderShellcodeEnd() {}
#pragma optimize( "", on )
#pragma runtime_checks( "", restore )

    unsigned long FindTargetPid(const std::string& app) {
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap == INVALID_HANDLE_VALUE) return 0;
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

    bool fetch_and_inject(const std::string& app_id, const std::string& token) {
        HMODULE hUrlMon = LoadLibraryA("urlmon.dll");
        if (!hUrlMon) return false;

        typedef HRESULT(WINAPI* URLDownloadToFileA_t)(void*, LPCSTR, LPCSTR, DWORD, void*);
        auto pURLDownloadToFileA = (URLDownloadToFileA_t)GetProcAddress(hUrlMon, "URLDownloadToFileA");

        bool bSuccess = false;
        if (pURLDownloadToFileA) {
            char tempPath[MAX_PATH];
            GetTempPathA(MAX_PATH, tempPath);
            std::string path = std::string(tempPath) + "weave_" + app_id + ".bin";
            
            std::string url = "http://localhost:3000/api/v1/loader/payload/" + app_id;
            if (pURLDownloadToFileA(NULL, url.c_str(), path.c_str(), 0, NULL) == S_OK) {
                // Read DLL
                HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD fileSize = GetFileSize(hFile, NULL);
                    std::vector<BYTE> dllData(fileSize);
                    DWORD bytesRead;
                    ReadFile(hFile, dllData.data(), fileSize, &bytesRead, NULL);
                    CloseHandle(hFile);

                    // Find Target (fallback to current process if game not found)
                    std::string targetProc = app_id + ".exe";
                    if (app_id == "cs2") targetProc = "cs2.exe";
                    else if (app_id == "rust") targetProc = "RustClient.exe";
                    
                    DWORD pid = FindTargetPid(targetProc);
                    if (pid == 0) pid = GetCurrentProcessId(); // fallback for testing

                    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
                    if (hProc) {
                        IMAGE_DOS_HEADER* pDos = reinterpret_cast<IMAGE_DOS_HEADER*>(dllData.data());
                        IMAGE_NT_HEADERS* pNt = reinterpret_cast<IMAGE_NT_HEADERS*>(dllData.data() + pDos->e_lfanew);

                        // 1. Map memory in target
                        BYTE* pTargetBase = reinterpret_cast<BYTE*>(VirtualAllocEx(hProc, NULL, pNt->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
                        
                        // 2. Map Sections
                        WriteProcessMemory(hProc, pTargetBase, dllData.data(), pNt->OptionalHeader.SizeOfHeaders, nullptr);
                        IMAGE_SECTION_HEADER* pSec = IMAGE_FIRST_SECTION(pNt);
                        for (UINT i = 0; i < pNt->FileHeader.NumberOfSections; ++i, ++pSec) {
                            if (pSec->SizeOfRawData) {
                                WriteProcessMemory(hProc, pTargetBase + pSec->VirtualAddress, dllData.data() + pSec->PointerToRawData, pSec->SizeOfRawData, nullptr);
                            }
                        }

                        // 3. Setup Mapping Data & arguments
                        ManualMappingData data = {};
                        data.pLoadLibraryA = LoadLibraryA;
                        data.pGetProcAddress = GetProcAddress;
                        
                        HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
                        if (hNtDll) {
                            data.pRtlAddFunctionTable = reinterpret_cast<BOOLEAN(WINAPI*)(PRUNTIME_FUNCTION, DWORD, DWORD64)>(GetProcAddress(hNtDll, "RtlAddFunctionTable"));
                        }

                        data.pBase = pTargetBase;
                        strncpy_s(data.token, token.c_str(), sizeof(data.token) - 1);
                        strncpy_s(data.app_id, app_id.c_str(), sizeof(data.app_id) - 1);

                        // 4. Inject Shellcode and mapping data
                        BYTE* pMappingData = reinterpret_cast<BYTE*>(VirtualAllocEx(hProc, NULL, sizeof(ManualMappingData), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
                        WriteProcessMemory(hProc, pMappingData, &data, sizeof(ManualMappingData), nullptr);

                        DWORD shellcodeSize = reinterpret_cast<DWORD_PTR>(LoaderShellcodeEnd) - reinterpret_cast<DWORD_PTR>(LoaderShellcode);
                        BYTE* pShellcode = reinterpret_cast<BYTE*>(VirtualAllocEx(hProc, NULL, shellcodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
                        WriteProcessMemory(hProc, pShellcode, LoaderShellcode, shellcodeSize, nullptr);

                        // 5. Execute
                        HANDLE hThread = CreateRemoteThread(hProc, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellcode), pMappingData, 0, NULL);
                        if (hThread) {
                            CloseHandle(hThread);
                            bSuccess = true;
                        }

                        CloseHandle(hProc);
                    }
                }
                DeleteFileA(path.c_str());
            }
        }
        FreeLibrary(hUrlMon);
        return bSuccess;
    }
}