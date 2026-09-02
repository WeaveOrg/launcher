#pragma once
#include <string>

namespace loader {
    std::string get_stage_name();
    int get_stage_progress();
    bool is_finished();
    bool was_successful();
    void set_stage(const std::string& name, int progress);
    void set_finished(bool finished);

    // Returns the raw OrionError code from the last injection attempt.
    int get_error_code();

    // Returns a human-readable description of the last OrionError.
    // Returns an empty string when there is no error (ORION_ERROR_NONE).
    std::string get_error_string();

    bool fetch_and_inject(const std::string& app_id, const std::string& token);
}
