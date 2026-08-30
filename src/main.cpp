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
#include "WebView2.h"
#include "hwid.hpp"
#include "process_manager.hpp"
#include "loader.hpp"
#undef UNICODE
#undef _UNICODE

#include <saucer/smartview.hpp>
#include <saucer/modules/loop.hpp>
#include <saucer/window.hpp>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
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

    // Backend payload fetch and inject logic (using new dedicated component)
    webview.expose("fetch_and_inject", [](const std::string& app_id, const std::string& token) -> bool {
        return loader::fetch_and_inject(app_id, token);
    });

    webview.set_url("http://localhost:3001");
    window->show();
    
    loop.run();
    return 0;
}
