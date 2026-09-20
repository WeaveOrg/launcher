#pragma once

#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

namespace loader {

enum class HiddenProcessResult {
  completed,
  start_failed,
  timed_out,
};

inline HiddenProcessResult
run_hidden_process(const std::filesystem::path &executable,
                   const std::wstring &arguments,
                   std::chrono::milliseconds timeout) noexcept {
  try {
    std::wstring command_line = L"\"" + executable.wstring() + L"\"";
    if (!arguments.empty()) {
      command_line += L" ";
      command_line += arguments;
    }
    std::vector<wchar_t> mutable_command_line(command_line.begin(),
                                               command_line.end());
    mutable_command_line.push_back(L'\0');

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESHOWWINDOW;
    startup_info.wShowWindow = SW_HIDE;

    PROCESS_INFORMATION process_info{};
    if (!CreateProcessW(executable.c_str(), mutable_command_line.data(), nullptr,
                        nullptr, FALSE,
                        CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, nullptr,
                        nullptr, &startup_info, &process_info)) {
      return HiddenProcessResult::start_failed;
    }

    CloseHandle(process_info.hThread);

    const auto timeout_count = std::clamp<std::int64_t>(
        timeout.count(), 0,
        static_cast<std::int64_t>((std::numeric_limits<DWORD>::max)()) -
            1LL);
    const DWORD wait_result =
        WaitForSingleObject(process_info.hProcess,
                            static_cast<DWORD>(timeout_count));

    if (wait_result == WAIT_TIMEOUT) {
      TerminateProcess(process_info.hProcess, ERROR_TIMEOUT);
      CloseHandle(process_info.hProcess);
      return HiddenProcessResult::timed_out;
    }

    CloseHandle(process_info.hProcess);
    return wait_result == WAIT_OBJECT_0 ? HiddenProcessResult::completed
                                       : HiddenProcessResult::start_failed;
  } catch (...) {
    return HiddenProcessResult::start_failed;
  }
}

}  // namespace loader
