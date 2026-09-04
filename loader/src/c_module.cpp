#include "c_module.hpp"
#include "manualmap.h"

#include <http2client/http2client_easy.h>

#include <synchapi.h>
#include <windows.h>

#include <atomic>
#include <format>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// CDN authority
// ---------------------------------------------------------------------------

#ifdef _DEBUG
static constexpr const char *CDN_AUTHORITY = "http://localhost:3000";
#else
static constexpr const char *CDN_AUTHORITY = "https://cdn.weave.su";
#endif

// ---------------------------------------------------------------------------
// Stage tracking
// ---------------------------------------------------------------------------

static std::mutex g_module_stage_mutex;
static std::string g_module_stage_name = "Ready";
static int g_module_stage_progress = 0;
static bool g_module_is_finished = false;
static void(WINAPI *g_report_stage)(const char *, int) = nullptr;

void c_module::set_stage(const std::string &name, int progress) {
  {
    std::lock_guard<std::mutex> lock(g_module_stage_mutex);
    g_module_stage_name = name;
    g_module_stage_progress = progress;
  }
  if (g_report_stage)
    g_report_stage(name.c_str(), progress);
}

void c_module::set_finished(bool finished) {
  std::lock_guard<std::mutex> lock(g_module_stage_mutex);
  g_module_is_finished = finished;
}

std::string c_module::get_stage_name() {
  std::lock_guard<std::mutex> lock(g_module_stage_mutex);
  return g_module_stage_name;
}

int c_module::get_stage_progress() {
  std::lock_guard<std::mutex> lock(g_module_stage_mutex);
  return g_module_stage_progress;
}

bool c_module::is_finished() {
  std::lock_guard<std::mutex> lock(g_module_stage_mutex);
  return g_module_is_finished;
}

// ---------------------------------------------------------------------------
// c_module
// ---------------------------------------------------------------------------

c_module &c_module::instance() {
  static c_module inst;
  return inst;
}

bool c_module::init(ManualMappingData *pData) {
  m_data = pData;
  if (!pData)
    return false;
  g_report_stage = pData->pReportStage;
  bool success = run(pData->token, pData->app_id);
  if (pData->pReportFinished)
    pData->pReportFinished(success ? TRUE : FALSE);
  return success;
}

bool c_module::run(const std::string &token, const std::string &app_id) {
  set_finished(false);

  // ── 1. Download loader.dll from CDN ──────────────────────────────────────
  // Orion itself handles CS2 launch, process injection, and waiting for libs.
  const std::string authority = "https://cdn.orion-security.pro";

  http2client::EasyClient cdn(authority);
  cdn.Bearer(token).Timeout(180000);

  set_stage("Download Library", 0);

  // Download with live progress reporting
  int download_progress = 0;
  auto r =
      cdn.GetWithProgress("/loader.dll", [&](std::size_t downloaded,
                                             std::optional<std::size_t> total) {
        if (!total.has_value())
          return;
        download_progress =
            static_cast<int>(static_cast<float>(downloaded) /
                             static_cast<float>(total.value()) * 100.f);
        set_stage(std::format("Download Library ({}%)", download_progress),
                  download_progress);
      });

  if (!r.ok()) {
    set_stage(std::format("Failed to download loader.dll from {}", authority),
              0);
    Sleep(2000);
    return false;
  }

  set_stage("Loading Library", 0);
  Sleep(200);

  // ── 4. Manual-map loader.dll in current process ───────────────────────────
  ManualMappedDll mappedDll;
  auto buffer = std::vector<char>(r.body.begin(), r.body.end());

  if (int err = mappedDll.Load(buffer); err != 0) {
    set_stage(std::format("Failed to map loader.dll [{}]: {}", err,
                          OrionErrorToString(static_cast<OrionError>(err))),
              0);
    Sleep(2000);
    return false;
  }

  set_stage("loader.dll mapped, initializing...", 75);
  Sleep(200);

  // ── 5. Build OrionData and invoke entry point ─────────────────────────────
  OrionData data{};
  data.token_or_key = const_cast<char *>(token.c_str());
  data.is_token = true;

  try {
    data.product_id = std::stoi(app_id);
  } catch (...) {
    data.product_id = 0;
  }

  data.progress = 0;
  data.error = 0;

  // Mirror data.progress into our stage bar on a background thread
  std::atomic<bool> progress_done{false};
  std::thread progress_thread([&]() {
    while (!progress_done.load()) {
      int p = data.progress;
      set_stage("Loading Library", p);
      if (p >= 100)
        break;
      Sleep(200);
    }
  });

  set_stage("Invoking loader.dll entry point...", 76);
  Sleep(1000);
  int invoke_error = mappedDll.InvokeMainFunction(&data);

  progress_done.store(true);
  if (progress_thread.joinable())
    progress_thread.join();

  if (invoke_error != 0 || data.error != 0) {
    int err = (data.error != 0) ? data.error : invoke_error;
    set_stage(std::format("loader.dll failed [{}]: {}", err,
                          OrionErrorToString(static_cast<OrionError>(err))),
              0);
    Sleep(2000);
    return false;
  }

  set_stage("Payload injected into CS2!", 100);
  set_finished(true);
  return true;
}
