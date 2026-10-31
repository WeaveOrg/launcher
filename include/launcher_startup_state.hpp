#pragma once

#include <cstdint>
#include <optional>

namespace launcher {

// Navigation and frontend readiness can arrive in either order. Readiness
// belongs to one navigation, and revealing the window is a one-time action.
class StartupState {
public:
    void navigation_started(std::uint64_t id) {
        if (shown_ || navigation_id_ == id) return;
        navigation_id_ = id;
        navigation_ok_ = false;
        frontend_ready_ = false;
    }

    bool navigation_completed(std::uint64_t id, bool success) {
        if (shown_ || navigation_id_ != id) return false;
        navigation_ok_ = success;
        if (!success) frontend_ready_ = false;
        return true;
    }

    void frontend_ready() {
        if (navigation_id_) frontend_ready_ = true;
    }

    bool take_show() {
        if (shown_ || !navigation_ok_ || !frontend_ready_) return false;
        shown_ = true;
        return true;
    }

private:
    std::optional<std::uint64_t> navigation_id_;
    bool navigation_ok_ = false;
    bool frontend_ready_ = false;
    bool shown_ = false;
};

} // namespace launcher
