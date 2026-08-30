#define UNICODE
#define _UNICODE
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <dwmapi.h>
#include <shlobj.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include "WebView2.h"
#include "hwid.hpp"
#include "process_manager.hpp"
#include "loader.hpp"
#undef UNICODE
#undef _UNICODE

#include <saucer/smartview.hpp>
#include <saucer/modules/loop.hpp>
#include <saucer/window.hpp>

#ifdef _DEBUG
#define LAUNCHER_URL "http://localhost:3000"
#else
#define LAUNCHER_URL "http://launcher.weave.su"
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow) {
    std::string token((const char*)(std::uintptr_t(hInstance) + 0x4f), 32);
    std::string target_url = std::string(LAUNCHER_URL) + "?token=" + token;

    auto app = saucer::application::create({.id = "weave_launcher"}).value();
    auto loop = saucer::modules::loop{app};

    // Use loop.application() to get the app pointer since operator& is deleted
    auto window = saucer::window::create(loop.application()).value();
    
    // smartview::create returns basic_smartview object
    auto webview = saucer::smartview::create({.window = window}).value();

    window->set_title("Weave Launcher");
    window->set_size({720, 450});
    window->set_min_size({720, 450});
    window->set_decorations(saucer::window::decoration::partial);
    window->set_background(saucer::color{0, 0, 0, 255}); // Black background

    // Expose stage state (std::string and int) to frontend
    webview.expose("get_stage_name", []() -> std::string {
        return loader::get_stage_name();
    });

    webview.expose("get_stage_progress", []() -> int {
        return loader::get_stage_progress();
    });

    // Backend payload fetch and inject logic
    webview.expose("fetch_and_inject", [](const std::string& app_id, const std::string& token) -> bool {
        return loader::fetch_and_inject(app_id, token);
    });

    webview.set_url(target_url);
    
    window->show();
    
    loop.run();
    return 0;
}
