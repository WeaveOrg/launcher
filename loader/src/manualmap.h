#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <vector>

#include "orionerror.h"

class ManualMappedDll {
public:
    ManualMappedDll() : m_pDosHeader(nullptr), m_pNtHeaders(nullptr),
                        m_pModuleBase(nullptr), m_pFileBuffer(nullptr) {}
    ~ManualMappedDll() {
        Cleanup();
    }

    // Loads a DLL from raw bytes into the current process memory.
    // Returns 0 on success, or an OrionError code on failure.
    int Load(const std::vector<char>& dllData);

    // Calls DllMain(DLL_PROCESS_ATTACH) and the obfuscated export,
    // then DllMain(DLL_PROCESS_DETACH) and frees the image.
    // Returns 0 on success, or an OrionError code on failure.
    int InvokeMainFunction(void* internal_data);

private:
    void Cleanup();

    // Internal loading steps
    int    ValidateHeaders(const char* pFileBuffer);
    bool   MapSections();
    int    ResolveImports();
    bool   ApplyRelocations();
    int    SetupExceptionHandling();
    FARPROC FindObfuscatedExport();

    // Pointers to key PE structures
    PIMAGE_DOS_HEADER m_pDosHeader;
    PIMAGE_NT_HEADERS m_pNtHeaders;
    char*             m_pModuleBase;  // Allocated memory for the mapped image
    const char*       m_pFileBuffer;  // Pointer to the raw DLL bytes (not owned)
};
