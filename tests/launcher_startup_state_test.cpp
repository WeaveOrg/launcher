#include "launcher_startup_state.hpp"

#include <cstdlib>
#include <iostream>

void require(bool value, const char* message) {
    if (value) return;
    std::cerr << "FAILED: " << message << '\n';
    std::exit(EXIT_FAILURE);
}

int main() {
    launcher::StartupState state;
    require(!state.take_show(), "window starts hidden");
    state.navigation_started(1);
    state.frontend_ready();
    require(!state.take_show(), "frontend readiness cannot reveal a pending navigation");
    state.navigation_completed(1, false);
    require(!state.take_show(), "network failure stays hidden even with a ready DOM");

    state.navigation_started(2);
    require(!state.navigation_completed(1, true), "stale completion is ignored");
    state.navigation_completed(2, true);
    require(!state.take_show(), "new navigation does not inherit old frontend readiness");
    state.frontend_ready();
    require(state.take_show(), "ready frontend and successful navigation reveal window");
    state.frontend_ready();
    require(!state.take_show(), "duplicate readiness cannot restore a minimized window");
    state.navigation_started(3);
    state.navigation_completed(3, true);
    require(!state.take_show(), "later navigations cannot reveal window again");

    launcher::StartupState early_frontend;
    early_frontend.navigation_started(10);
    early_frontend.frontend_ready();
    early_frontend.navigation_started(10);
    early_frontend.navigation_completed(10, true);
    require(early_frontend.take_show(), "redirects preserve readiness for the same navigation");

    launcher::StartupState replacement;
    replacement.navigation_started(20);
    replacement.frontend_ready();
    replacement.navigation_started(21);
    replacement.navigation_completed(21, true);
    require(!replacement.take_show(), "replacement navigation requires a fresh frontend signal");
    std::cout << "launcher startup state tests passed\n";
}
