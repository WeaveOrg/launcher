#pragma once

#include <string>

namespace Weave {
namespace Hardware {

/**
 * @brief Generates a unique, reproducible Hardware ID for the local Windows machine.
 * Combines CPU ID, Machine GUID, and Volume Serial Number into a SHA-256 hash.
 */
std::string GenerateHWID();

/**
 * @brief Retrieves the CPU brand string (e.g. "AMD Ryzen 7 7800X3D 8-Core Processor").
 */
std::string GetCPUName();

/**
 * @brief Retrieves the Primary GPU name from Windows registry / DXGI.
 */
std::string GetGPUName();

/**
 * @brief Retrieves total physical RAM in Gigabytes.
 */
int GetTotalRAMGigabytes();

/**
 * @brief Retrieves Windows OS version string.
 */
std::string GetOSVersion();

} // namespace Hardware
} // namespace Weave
