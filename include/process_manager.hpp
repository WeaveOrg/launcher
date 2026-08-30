#pragma once

#include <string>
#include <vector>

namespace Weave {
namespace Process {

struct ProcessInfo {
    unsigned long pid;
    std::wstring name;
};

/**
 * @brief Checks whether a specific process name (e.g. "cs2.exe") is currently running.
 */
bool IsProcessRunning(const std::wstring& processName);

/**
 * @brief Finds process ID by executable name.
 */
unsigned long FindProcessId(const std::wstring& processName);

/**
 * @brief Launches an executable process with arguments and returns process status string.
 */
std::string LaunchTargetProcess(const std::string& appId, const std::string& processName, const std::string& arguments);

/**
 * @brief Simulates safe memory mapping / remote thread execution for loader injection.
 */
std::string InjectModule(unsigned long pid, const std::string& modulePayloadPath);

} // namespace Process
} // namespace Weave
