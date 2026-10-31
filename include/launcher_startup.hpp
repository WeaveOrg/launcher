#pragma once

#include "launcher_startup_state.hpp"

#include <saucer/smartview.hpp>
#include <saucer/modules/stable/webview2.hpp>
#include <commctrl.h>
#include <wrl.h>

namespace launcher {

// Owns WebView2 subscriptions and the UI-thread retry timer. Construct after
// the smartview so subscriptions are removed before its controller is closed.
class Startup {
public:
    Startup(saucer::smartview& webview, std::shared_ptr<saucer::window> window,
            std::string target_url)
        : webview_(webview), window_(std::move(window)), target_url_(std::move(target_url)),
          core_(webview.native<true>().webview), hwnd_(window_->native<true>().hwnd) {}

    Startup(const Startup&) = delete;
    Startup& operator=(const Startup&) = delete;

    ~Startup() {
        KillTimer(hwnd_, timer_id());
        if (hooked_) RemoveWindowSubclass(hwnd_, &window_proc, timer_id());
        if (started_registered_) core_->remove_NavigationStarting(started_token_);
        if (completed_registered_) core_->remove_NavigationCompleted(completed_token_);
    }

    HRESULT initialize() {
        Microsoft::WRL::ComPtr<ICoreWebView2Settings> settings;
        HRESULT hr = core_->get_Settings(&settings);
        if (FAILED(hr)) return hr;
        hr = settings->put_IsBuiltInErrorPageEnabled(FALSE);
        if (FAILED(hr)) return hr;

        if (!SetWindowSubclass(hwnd_, &window_proc, timer_id(), reinterpret_cast<DWORD_PTR>(this))) {
            return E_FAIL;
        }
        hooked_ = true;

        hr = core_->add_NavigationStarting(
            Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
                [this](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                    UINT64 id{};
                    const HRESULT result = args->get_NavigationId(&id);
                    if (SUCCEEDED(result)) {
                        KillTimer(hwnd_, timer_id());
                        state_.navigation_started(id);
                    }
                    return S_OK;
                }).Get(), &started_token_);
        if (FAILED(hr)) return hr;
        started_registered_ = true;

        hr = core_->add_NavigationCompleted(
            Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                [this](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                    UINT64 id{};
                    BOOL success = FALSE;
                    if (FAILED(args->get_NavigationId(&id))) return S_OK;
                    args->get_IsSuccess(&success);

                    // An HTTP error document is not the launcher, even if the
                    // runtime considers its navigation complete.
                    Microsoft::WRL::ComPtr<ICoreWebView2NavigationCompletedEventArgs2> response;
                    if (SUCCEEDED(args->QueryInterface(IID_PPV_ARGS(&response)))) {
                        int status{};
                        if (SUCCEEDED(response->get_HttpStatusCode(&status)) && status >= 400) success = FALSE;
                    }
                    if (!state_.navigation_completed(id, success != FALSE)) return S_OK;
                    if (success) show_if_ready();
                    else schedule_retry();
                    return S_OK;
                }).Get(), &completed_token_);
        if (FAILED(hr)) return hr;
        completed_registered_ = true;
        return S_OK;
    }

    bool frontend_ready() {
        const auto url = webview_.url();
        if (url.scheme() != "https" || url.host() != "launcher.weave.su") return false;
        state_.frontend_ready();
        show_if_ready();
        return true;
    }

private:
    UINT_PTR timer_id() const { return reinterpret_cast<UINT_PTR>(this); }

    void schedule_retry() {
        SetTimer(hwnd_, timer_id(), 3000, nullptr);
    }

    void show_if_ready() {
        if (!state_.take_show()) return;
        KillTimer(hwnd_, timer_id());
        window_->show();
    }

    static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam,
                                        LPARAM lparam, UINT_PTR id, DWORD_PTR data) {
        auto* self = reinterpret_cast<Startup*>(data);
        if (message == WM_TIMER && wparam == id) {
            KillTimer(hwnd, id);
            self->webview_.set_url(self->target_url_);
            return 0;
        }
        if (message == WM_NCDESTROY) {
            KillTimer(hwnd, id);
            RemoveWindowSubclass(hwnd, &window_proc, id);
            self->hooked_ = false;
        }
        return DefSubclassProc(hwnd, message, wparam, lparam);
    }

    saucer::smartview& webview_;
    std::shared_ptr<saucer::window> window_;
    std::string target_url_;
    Microsoft::WRL::ComPtr<ICoreWebView2> core_;
    HWND hwnd_;
    StartupState state_;
    EventRegistrationToken started_token_{};
    EventRegistrationToken completed_token_{};
    bool hooked_ = false;
    bool started_registered_ = false;
    bool completed_registered_ = false;
};

} // namespace launcher
