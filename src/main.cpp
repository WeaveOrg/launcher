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

// Helper to determine the target frontend URL
std::string get_target_url(const std::string& cmd_line) {
    // 1. If command line contains a full URL (http:// or https://)
    if (cmd_line.find("http://") != std::string::npos || cmd_line.find("https://") != std::string::npos) {
        size_t start_pos = cmd_line.find("http");
        size_t end_pos = cmd_line.find(' ', start_pos);
        if (end_pos == std::string::npos) {
            return cmd_line.substr(start_pos);
        }
        return cmd_line.substr(start_pos, end_pos - start_pos);
    }

    // 2. Check environment variable for remote or custom URL
    char env_buffer[1024];
    DWORD env_len = GetEnvironmentVariableA("WEAVE_LAUNCHER_URL", env_buffer, sizeof(env_buffer));
    std::string base_url = (env_len > 0) ? std::string(env_buffer) : "http://localhost:3000";

    // 3. If command line has a token or ?token=...
    std::string clean_cmd = cmd_line;
    // Trim whitespace
    clean_cmd.erase(0, clean_cmd.find_first_not_of(" \t\r\n\""));
    clean_cmd.erase(clean_cmd.find_last_not_of(" \t\r\n\"") + 1);

    if (!clean_cmd.empty()) {
        if (clean_cmd.find("?token=") != std::string::npos) {
            return base_url + (clean_cmd.front() == '?' ? clean_cmd : ("/" + clean_cmd));
        }
        if (clean_cmd.find("--token=") == 0) {
            std::string token = clean_cmd.substr(8);
            return base_url + "?token=" + token;
        }
        if (clean_cmd.find("--token") == 0) {
            size_t val_pos = clean_cmd.find_first_not_of(" \t", 7);
            if (val_pos != std::string::npos) {
                return base_url + "?token=" + clean_cmd.substr(val_pos);
            }
        }
        // If raw token passed: launcher.exe f3aa10c6a45a48a77506ea4c37ce2b57
        if (clean_cmd.find('/') == std::string::npos && clean_cmd.find('\\') == std::string::npos) {
            return base_url + "?token=" + clean_cmd;
        }
    }

    return base_url;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int nCmdShow) {
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

    // Backend payload fetch and inject logic
    webview.expose("fetch_and_inject", [](const std::string& app_id, const std::string& token) -> bool {
        return loader::fetch_and_inject(app_id, token);
    });

    // Resolve URL from command line argument, environment variable, or fallback
    std::string target_url = get_target_url(lpCmdLine ? lpCmdLine : "");
    webview.set_url(target_url);
    
    window->show();
    
    loop.run();
    return 0;
}
