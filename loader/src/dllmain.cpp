#include "c_module.hpp"
#include <winuser.h>

DWORD WINAPI ModuleInitThread(LPVOID lpParam) {
  ManualMappingData *pData = reinterpret_cast<ManualMappingData *>(lpParam);
  if (pData) {
    c_module::instance().init(pData);
    delete pData;
  }
  return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hinstDLL);

    if (lpvReserved != nullptr) {
      ManualMappingData *pData =
          reinterpret_cast<ManualMappingData *>(lpvReserved);
      ManualMappingData *pCopy = new ManualMappingData(*pData);

      HANDLE hThread = CreateThread(nullptr, 0, ModuleInitThread, pCopy, 0, nullptr);
      if (hThread) {
        CloseHandle(hThread);
      }
    }
  }
  return TRUE;
}