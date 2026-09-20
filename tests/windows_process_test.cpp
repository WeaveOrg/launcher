#include "windows_process.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

namespace {

void require(bool condition, const char *message) {
  if (condition)
    return;
  std::cerr << "FAILED: " << message << '\n';
  std::exit(EXIT_FAILURE);
}

std::filesystem::path system_executable(const wchar_t *name) {
  wchar_t directory[MAX_PATH]{};
  const UINT length = GetSystemDirectoryW(directory, MAX_PATH);
  require(length > 0 && length < MAX_PATH,
          "Windows system directory is available");
  return std::filesystem::path(directory) / name;
}

std::filesystem::path current_executable() {
  std::wstring path(32768, L'\0');
  const DWORD length =
      GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
  require(length > 0 && length < path.size(),
          "current executable path is available");
  path.resize(length);
  return path;
}

std::filesystem::path console_report_path(std::string_view process_id) {
  return std::filesystem::temp_directory_path() /
         ("weave-hidden-process-" + std::string(process_id) + ".txt");
}

void reports_completed_process() {
  const auto result = loader::run_hidden_process(
      system_executable(L"cmd.exe"), L"/c exit 0", std::chrono::seconds(2));

  require(result == loader::HiddenProcessResult::completed,
          "completed process is reported as completed");
}

void reports_missing_executable_without_throwing() {
  const auto result = loader::run_hidden_process(
      system_executable(L"definitely-missing-weave-test.exe"), L"",
      std::chrono::seconds(2));

  require(result == loader::HiddenProcessResult::start_failed,
          "missing executable is reported as a startup failure");
}

void stops_waiting_after_timeout() {
  const auto started = std::chrono::steady_clock::now();
  const auto result = loader::run_hidden_process(
      system_executable(L"cmd.exe"), L"/c ping 127.0.0.1 -n 4 > nul",
      std::chrono::milliseconds(20));
  const auto elapsed = std::chrono::steady_clock::now() - started;

  require(result == loader::HiddenProcessResult::timed_out,
          "long-running process is reported as timed out");
  require(elapsed < std::chrono::milliseconds(500),
          "timeout does not add a second blocking wait");
}

void starts_process_without_a_console_window() {
  const bool owns_test_console = GetConsoleWindow() == nullptr;
  if (owns_test_console) {
    FreeConsole();
    require(AllocConsole() != FALSE, "test console can be allocated");
    ShowWindow(GetConsoleWindow(), SW_HIDE);
  }

  const std::string process_id = std::to_string(GetCurrentProcessId());
  const auto report_path = console_report_path(process_id);
  std::error_code error;
  std::filesystem::remove(report_path, error);

  const std::wstring arguments =
      L"--report-console " + std::to_wstring(GetCurrentProcessId());
  const auto result = loader::run_hidden_process(
      current_executable(), arguments, std::chrono::seconds(2));

  require(result == loader::HiddenProcessResult::completed,
          "console-reporting child completes");
  std::ifstream report(report_path);
  std::string console_state;
  report >> console_state;
  std::filesystem::remove(report_path, error);
  if (owns_test_console)
    FreeConsole();
  require(console_state == "hidden", "child process has no console window");
}

}  // namespace

int main(int argc, char **argv) {
  if (argc == 3 && std::string_view(argv[1]) == "--report-console") {
    std::ofstream report{console_report_path(argv[2])};
    report << (GetConsoleWindow() == nullptr ? "hidden" : "visible");
    return report ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  reports_completed_process();
  reports_missing_executable_without_throwing();
  stops_waiting_after_timeout();
  starts_process_without_a_console_window();
  std::cout << "windows_process_test: PASS\n";
}
