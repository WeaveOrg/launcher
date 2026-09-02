#include "manualmap.h"

#include <vector>
#include <iostream>

// ---------------------------------------------------------------------------
// Load
// ---------------------------------------------------------------------------

int ManualMappedDll::Load(const std::vector<char>& dllData) {
    m_pFileBuffer = dllData.data();

    // --- 3. Validate PE headers ---
    auto ret = ValidateHeaders(m_pFileBuffer);
    if (ret != 0) {
        return ret;
    }

    // --- 4. Allocate memory for the DLL image ---
    m_pModuleBase = reinterpret_cast<char*>(VirtualAlloc(
        nullptr,
        m_pNtHeaders->OptionalHeader.SizeOfImage,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    ));

    if (!m_pModuleBase) {
        return OrionError::ORION_ERROR_ALLOCATE_MEMORY_1;
    }

    // --- 5. Copy headers and sections ---
    if (!MapSections()) {
        Cleanup();
        return OrionError::ORION_ERROR_ALLOCATE_MEMORY_2;
    }

    // --- 6. Resolve imports ---
    ret = ResolveImports();
    if (ret != 0) {
        Cleanup();
        return ret;
    }

    // --- 7. Apply relocations ---
    if (!ApplyRelocations()) {
        Cleanup();
        return OrionError::ORION_ERROR_INVALID_DATA;
    }

    // --- 8. Setup exception handling (SEH) ---
    ret = SetupExceptionHandling();
    if (ret != 0) {
        Cleanup();
        return ret;
    }

    return 0;
}

// ---------------------------------------------------------------------------
// ValidateHeaders
// ---------------------------------------------------------------------------

int ManualMappedDll::ValidateHeaders(const char* pFileBuffer) {
    m_pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<char*>(pFileBuffer));
    if (m_pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return OrionError::ORION_ERROR_INVALID_MODULE_1;
    }

    m_pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<char*>(pFileBuffer) + m_pDosHeader->e_lfanew);
    if (m_pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        return OrionError::ORION_ERROR_INVALID_MODULE_2;
    }

    if (m_pNtHeaders->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        return OrionError::ORION_ERROR_INVALID_MODULE_3;
    }

    if (m_pNtHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return OrionError::ORION_ERROR_INVALID_MODULE_4;
    }

    if (m_pNtHeaders->FileHeader.NumberOfSections == 0)
        return OrionError::ORION_ERROR_INVALID_MODULE_5;

    if (m_pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0)
        return OrionError::ORION_ERROR_INVALID_MODULE_6;

    if (m_pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size == 0)
        return OrionError::ORION_ERROR_INVALID_MODULE_7;

    if (m_pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size == 0)
        return OrionError::ORION_ERROR_INVALID_MODULE_8;

    return 0;
}

// ---------------------------------------------------------------------------
// MapSections
// ---------------------------------------------------------------------------

bool ManualMappedDll::MapSections() {
    // Copy headers
    memcpy(m_pModuleBase, m_pFileBuffer, m_pNtHeaders->OptionalHeader.SizeOfHeaders);

    // Copy each section
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(m_pNtHeaders);
    for (WORD i = 0; i < m_pNtHeaders->FileHeader.NumberOfSections; ++i, ++pSectionHeader) {
        if (pSectionHeader->SizeOfRawData == 0)
            continue;
        memcpy(
            m_pModuleBase + pSectionHeader->VirtualAddress,
            m_pFileBuffer + pSectionHeader->PointerToRawData,
            pSectionHeader->SizeOfRawData
        );
    }
    return true;
}

// ---------------------------------------------------------------------------
// ResolveImports
// ---------------------------------------------------------------------------

int ManualMappedDll::ResolveImports() {
    auto importDir = m_pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (importDir.Size == 0) {
        return 0; // No imports
    }

    PIMAGE_IMPORT_DESCRIPTOR pImportDesc =
        reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(m_pModuleBase + importDir.VirtualAddress);

    while (pImportDesc->Name) {
        char* dllName = m_pModuleBase + pImportDesc->Name;
        HMODULE hModule = LoadLibraryA(dllName);

        if (!hModule) {
            return OrionError::ORION_ERROR_LOAD_LIBRARY_1;
        }

        PIMAGE_THUNK_DATA pThunk =
            reinterpret_cast<PIMAGE_THUNK_DATA>(m_pModuleBase + pImportDesc->FirstThunk);
        PIMAGE_THUNK_DATA pOrigThunk =
            reinterpret_cast<PIMAGE_THUNK_DATA>(m_pModuleBase + pImportDesc->OriginalFirstThunk);

        if (!pOrigThunk) pOrigThunk = pThunk;

        while (pOrigThunk->u1.AddressOfData) {
            FARPROC pfn;
            if (IMAGE_SNAP_BY_ORDINAL(pOrigThunk->u1.Ordinal)) {
                LPCSTR ordinal = reinterpret_cast<LPCSTR>(IMAGE_ORDINAL(pOrigThunk->u1.Ordinal));
                pfn = GetProcAddress(hModule, ordinal);
                if (!pfn) {
                    return OrionError::ORION_ERROR_GET_FUNCTION_ADDRESS_1;
                }
            } else {
                PIMAGE_IMPORT_BY_NAME pImportByName =
                    reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(m_pModuleBase + pOrigThunk->u1.AddressOfData);
                pfn = GetProcAddress(hModule, pImportByName->Name);
                if (!pfn) {
                    return OrionError::ORION_ERROR_GET_FUNCTION_ADDRESS_2;
                }
            }

            pThunk->u1.Function = reinterpret_cast<ULONGLONG>(pfn);
            pThunk++;
            pOrigThunk++;
        }
        pImportDesc++;
    }

    return 0;
}

// ---------------------------------------------------------------------------
// ApplyRelocations
// ---------------------------------------------------------------------------

bool ManualMappedDll::ApplyRelocations() {
    auto relocDir = m_pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (relocDir.Size == 0) {
        return true; // No relocations
    }

    LONGLONG delta = reinterpret_cast<ULONGLONG>(m_pModuleBase) -
                     m_pNtHeaders->OptionalHeader.ImageBase;
    if (delta == 0) {
        return true; // Already at preferred address
    }

    PIMAGE_BASE_RELOCATION pRelocBlock =
        reinterpret_cast<PIMAGE_BASE_RELOCATION>(m_pModuleBase + relocDir.VirtualAddress);

    while (pRelocBlock->VirtualAddress) {
        if (pRelocBlock->SizeOfBlock < sizeof(IMAGE_BASE_RELOCATION))
            break;

        DWORD count = (pRelocBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
        WORD* pRelocEntry = reinterpret_cast<WORD*>(pRelocBlock + 1);

        for (DWORD i = 0; i < count; ++i, ++pRelocEntry) {
            WORD type   = (*pRelocEntry) >> 12;
            WORD offset = (*pRelocEntry) & 0xFFF;

            if (type == IMAGE_REL_BASED_DIR64) {
                ULONGLONG* pPatchAddr =
                    reinterpret_cast<ULONGLONG*>(m_pModuleBase + pRelocBlock->VirtualAddress + offset);
                *pPatchAddr += delta;
            }
        }
        pRelocBlock = reinterpret_cast<PIMAGE_BASE_RELOCATION>(
            reinterpret_cast<char*>(pRelocBlock) + pRelocBlock->SizeOfBlock);
    }
    return true;
}

// ---------------------------------------------------------------------------
// SetupExceptionHandling
// ---------------------------------------------------------------------------

int ManualMappedDll::SetupExceptionHandling() {
    auto excepDir = m_pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
    if (excepDir.Size == 0) {
        return 0; // No exception table
    }

    PRUNTIME_FUNCTION pRuntimeFunc =
        reinterpret_cast<PRUNTIME_FUNCTION>(m_pModuleBase + excepDir.VirtualAddress);
    DWORD entryCount = excepDir.Size / sizeof(RUNTIME_FUNCTION);

    if (!RtlAddFunctionTable(pRuntimeFunc, entryCount, reinterpret_cast<DWORD64>(m_pModuleBase))) {
        return OrionError::ORION_ERROR_SETUP_EXCEPTIONS;
    }

    return 0;
}

// ---------------------------------------------------------------------------
// FindObfuscatedExport
// ---------------------------------------------------------------------------

FARPROC ManualMappedDll::FindObfuscatedExport() {
    auto exportDirRVA =
        m_pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (!exportDirRVA) return nullptr;

    PIMAGE_EXPORT_DIRECTORY pExportDir =
        reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(m_pModuleBase + exportDirRVA);

    // Reproduces the obfuscated pointer chain from the decompiler:
    // v9 + *(v9 + *(v9 + rva_export_dir + 28))
    DWORD  rva_table_of_names = pExportDir->AddressOfFunctions;
    DWORD* pRvaOfSomething    = reinterpret_cast<DWORD*>(m_pModuleBase + rva_table_of_names);
    DWORD  rvaOfFunction      = *pRvaOfSomething;

    return reinterpret_cast<FARPROC>(m_pModuleBase + rvaOfFunction);
}

// ---------------------------------------------------------------------------
// InvokeMainFunction
// ---------------------------------------------------------------------------

int ManualMappedDll::InvokeMainFunction(void* internal_data) {
    // --- 9. Call DllMain for initialization ---
    DWORD entryPointRVA = m_pNtHeaders->OptionalHeader.AddressOfEntryPoint;
    if (entryPointRVA == 0) {
        return 0; // No entry point
    }

    using DllMain_t = BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID);
    DllMain_t pfnDllMain =
        reinterpret_cast<DllMain_t>(m_pModuleBase + entryPointRVA);

    if (!pfnDllMain(reinterpret_cast<HINSTANCE>(m_pModuleBase), DLL_PROCESS_ATTACH, nullptr)) {
        return OrionError::ORION_ERROR_START_MODULE;
    }

    // --- 10. Call the obfuscated main export ---
    using MainFunc_t = int(__fastcall*)(void*);
    FARPROC pfnMainFunc = FindObfuscatedExport();
    if (!pfnMainFunc) {
        pfnDllMain(reinterpret_cast<HINSTANCE>(m_pModuleBase), DLL_PROCESS_DETACH, nullptr);
        return 0;
    }

    int result = reinterpret_cast<MainFunc_t>(pfnMainFunc)(internal_data);

    // --- 11. Call DllMain for deinitialization ---
    BOOL ret = pfnDllMain(reinterpret_cast<HINSTANCE>(m_pModuleBase), DLL_PROCESS_DETACH, nullptr);
    BOOL l   = VirtualFree(m_pModuleBase, 0, MEM_RELEASE);
    m_pModuleBase = nullptr;

    if (!ret) {
        return OrionError::ORION_ERROR_STOP_MODULE;
    }

    if (!l) {
        return OrionError::ORION_ERROR_FREE_MEMORY_1;
    }

    return result;
}

// ---------------------------------------------------------------------------
// Cleanup
// ---------------------------------------------------------------------------

void ManualMappedDll::Cleanup() {
    if (m_pModuleBase) {
        VirtualFree(m_pModuleBase, 0, MEM_RELEASE);
        m_pModuleBase = nullptr;
    }
}
