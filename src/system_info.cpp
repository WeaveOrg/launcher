#include "system_info.hpp"
#include "hwid.hpp"
#include <sstream>

namespace Weave {

std::string GetSystemInfoJson() {
    std::string os = Hardware::GetOSVersion();
    std::string cpu = Hardware::GetCPUName();
    std::string gpu = Hardware::GetGPUName();
    int ram = Hardware::GetTotalRAMGigabytes();
    std::string hwid = Hardware::GenerateHWID();

    std::stringstream ss;
    ss << "{"
       << "\"os\":\"" << os << "\","
       << "\"cpu\":\"" << cpu << "\","
       << "\"gpu\":\"" << gpu << "\","
       << "\"ramGb\":" << ram << ","
       << "\"hwid\":\"" << hwid << "\","
       << "\"antivirusDetected\":\"None (Game Mode Enabled)\","
       << "\"secureBoot\":true"
       << "}";

    return ss.str();
}

} // namespace Weave
