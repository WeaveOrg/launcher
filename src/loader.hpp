#pragma once
#include <string>

namespace loader {
    std::string get_stage_name();
    int get_stage_progress();
    void set_stage(const std::string& name, int progress);
    bool fetch_and_inject(const std::string& app_id, const std::string& token);
}