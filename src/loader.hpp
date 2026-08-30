#pragma once
#include <string>

namespace loader {
    bool fetch_and_inject(const std::string& app_id, const std::string& token);
}