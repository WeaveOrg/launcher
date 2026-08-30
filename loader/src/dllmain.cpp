#include "c_module.hpp"
#include <winuser.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  if (fdwReason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hinstDLL);

    if (lpvReserved != nullptr) {
      ManualMappingData *pData =
          reinterpret_cast<ManualMappingData *>(lpvReserved);
      
      c_module::instance().init(pData);
    }
  }
  return TRUE;
}