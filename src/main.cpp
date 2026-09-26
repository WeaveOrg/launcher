#define UNICODE
#define _UNICODE
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <shellapi.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <filesystem>
#include <future>
#include <thread>
#include "WebView2.h"
#include "hwid.hpp"
#include "process_manager.hpp"
#include "loader.hpp"
#undef UNICODE
#undef _UNICODE

#include <saucer/smartview.hpp>
#include <saucer/modules/loop.hpp>
#include <saucer/window.hpp>
#include <saucer/modules/stable/webview2.hpp>

#define LAUNCHER_URL "https://launcher.weave.su"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Detect system DPI and scale window size accordingly
    int system_dpi = GetDpiForSystem();
    float dpi_scale = system_dpi / 96.0f;  // 96 DPI = 100%
    int window_width  = static_cast<int>(720 * dpi_scale);
    int window_height = static_cast<int>(450 * dpi_scale);

    std::string token((const char*)(std::uintptr_t(hInstance) + 0x4f), 32);
    std::string target_url = std::string(LAUNCHER_URL) + "?token=" + token;

    auto app = saucer::application::create({.id = "weave_launcher"}).value();
    auto loop = saucer::modules::loop{app};

    // Use loop.application() to get the app pointer since operator& is deleted
    auto window = saucer::window::create(loop.application()).value();
    
    // smartview::create returns basic_smartview object
    auto webview = saucer::smartview::create({.window = window}).value();

    window->set_title("Weave Launcher");
    window->set_size({window_width, window_height});
    window->set_min_size({window_width, window_height});
    window->set_resizable(false);
    window->set_decorations(saucer::window::decoration::partial);
    window->set_background(saucer::color{0, 0, 0, 255}); // Black background
    webview.set_context_menu(false);

    // Window/taskbar icon comes from the ICON resource compiled into the exe
    // (resources/app.rc, id 1), so it works for the standalone downloaded
    // binary and needs no app.ico next to it. We bypass saucer::window::set_icon:
    // it goes through GDI+, which only ever picks one ICO frame (the 16x16 one)
    // and sets ICON_BIG alone, so the taskbar ends up with an upscaled 16px
    // image. LoadImage lets Windows pick the best-matching frame per size.
    // The webview's favicon is deliberately not applied to the window.
    {
        const HWND hwnd = window->native<true>().hwnd;
        const HINSTANCE module = GetModuleHandleW(nullptr);
        const UINT dpi = GetDpiForWindow(hwnd);
        const int small_px = GetSystemMetricsForDpi(SM_CXSMICON, dpi);
        const int big_px = GetSystemMetricsForDpi(SM_CXICON, dpi);

        if (auto *small_icon = static_cast<HICON>(LoadImageW(module, MAKEINTRESOURCEW(1), IMAGE_ICON, small_px, small_px, LR_DEFAULTCOLOR | LR_SHARED))) {
            SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(small_icon));
        }
        if (auto *big_icon = static_cast<HICON>(LoadImageW(module, MAKEINTRESOURCEW(1), IMAGE_ICON, big_px, big_px, LR_DEFAULTCOLOR | LR_SHARED))) {
            SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(big_icon));
        }
    }

    // Expose stage state (std::string and int) to frontend
    webview.expose("get_stage_name", []() -> std::string {
        return loader::get_stage_name();
    });

    webview.expose("get_stage_progress", []() -> int {
        return loader::get_stage_progress();
    });

    webview.expose("is_finished", []() -> bool {
        return loader::is_finished();
    });

    webview.expose("was_successful", []() -> bool {
        return loader::was_successful();
    });

    // Expose OrionError details to the frontend for failure reporting
    webview.expose("get_error_code", []() -> int {
        return loader::get_error_code();
    });

    webview.expose("get_error_string", []() -> std::string {
        return loader::get_error_string();
    });

    // The stale-launcher screen can only open this fixed first-party URL.
    // ShellExecuteW delegates it to the user's default browser.
    webview.expose("open_dashboard", []() -> bool {
        const auto result = ShellExecuteW(
            nullptr,
            L"open",
            L"https://weave.su/dashboard/products",
            nullptr,
            nullptr,
            SW_SHOWNORMAL
        );
        return reinterpret_cast<INT_PTR>(result) > 32;
    });

    // Start asynchronous injection pipeline in dedicated background worker
    webview.expose("start_injection", [](const std::string& app_id, const std::string& token) -> bool {
        std::thread([app_id, token]() {
            loader::fetch_and_inject(app_id, token);
        }).detach();
        return true;
    });

    webview.expose("fetch_and_inject", [](const std::string& app_id, const std::string& token) -> bool {
        return loader::fetch_and_inject(app_id, token);
    });

    webview.set_url(target_url);
    
    window->show();
    
    loop.run();
    return 0;
}
