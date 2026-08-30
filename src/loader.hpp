#pragma once
#include <string>

namespace loader {
    std::string get_stage_name();
    int get_stage_progress();
    bool is_finished();
    void set_stage(const std::string& name, int progress);
    void set_finished(bool finished);

    // Phase 1: loader.dll CDN download (0-100%)
    std::string get_download_stage();
    int get_download_progress();

    // Phase 2: Manual mapping into target process (0-100%)
    std::string get_mmap_stage();
    int get_mmap_progress();

    bool fetch_and_inject(const std::string& app_id, const std::string& token);
}