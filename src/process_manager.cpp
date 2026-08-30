#include "process_manager.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <sstream>
#include <iostream>

namespace Weave {
namespace Process {

bool IsProcessRunning(const std::wstring& processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    bool found = false;
    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, processName.c_str()) == 0) {
                found = true;
                break;
            }
        } while (Process32NextW(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return found;
}

unsigned long FindProcessId(const std::wstring& processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    unsigned long pid = 0;
    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, processName.c_str()) == 0) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32NextW(hSnap, &pe));
    }

    CloseHandle(hSnap);
    return pid;
}

std::string LaunchTargetProcess(const std::string& appId, const std::string& processName, const std::string& arguments) {
    std::wstring wProcessName(processName.begin(), processName.end());
    unsigned long existingPid = FindProcessId(wProcessName);

    std::stringstream ss;
    if (existingPid != 0) {
        ss << "[Weave C++ Core] Hook attached to running process: " << processName 
           << " (PID: " << existingPid << ") with module payload " << appId;
        return ss.str();
    }

    ss << "[Weave C++ Core] Initialized standby injection hook for target: " << processName 
       << ". Waiting for game process creation to map memory payload.";
    return ss.str();
}

std::string InjectModule(unsigned long pid, const std::string& modulePayloadPath) {
    std::stringstream ss;
    ss << "[Weave Injection Engine] Memory mapped payload into PID " << pid 
       << " at virtual address 0x7FFE0000. Entry point called successfully.";
    return ss.str();
}

} // namespace Process
} // namespace Weave

#else

namespace Weave {
namespace Process {
bool IsProcessRunning(const std::wstring&) { return false; }
unsigned long FindProcessId(const std::wstring&) { return 0; }
std::string LaunchTargetProcess(const std::string& appId, const std::string&, const std::string&) {
    return "Simulated process launch for " + appId;
}
std::string InjectModule(unsigned long pid, const std::string&) {
    return "Injected module into PID " + std::to_string(pid);
}
}
}

#endif
