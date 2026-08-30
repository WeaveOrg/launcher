#pragma once

#include <string>

namespace Weave {

/**
 * @brief Generates formatted JSON containing OS, CPU, GPU, RAM, HWID and Security Status.
 */
std::string GetSystemInfoJson();

} // namespace Weave
