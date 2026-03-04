#include "oui_lookup.h"

#include <unordered_map>
#include <string>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <windows.h>
#include <vector>

static std::unordered_map<std::string, std::string> ouiDatabase;

static void addOuiRange(const std::vector<std::string>& prefixes, const std::string& vendor) {
    for (const auto& prefix : prefixes) {
        ouiDatabase[prefix] = vendor;
    }
}

void initOuiDatabase() {
    if (!ouiDatabase.empty()) return;

    // Try to load from file first
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) != 0) {
        std::string dir = path;
        size_t lastSlash = dir.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            dir = dir.substr(0, lastSlash);
        }
        
        std::string ouiFile = dir + "\\oui.txt";
        std::ifstream file(ouiFile);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.length() >= 8 && line[2] == ':' && line[5] == ':') {
                    std::string prefix = line.substr(0, 8);
                    std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper);
                    std::string vendor = line.substr(9);
                    // Trim whitespace
                    vendor.erase(0, vendor.find_first_not_of(" \t\r\n"));
                    size_t last = vendor.find_last_not_of(" \t\r\n");
                    if (last != std::string::npos) {
                        vendor.erase(last + 1);
                    }
                    ouiDatabase[prefix] = vendor;
                }
            }
            if (!ouiDatabase.empty()) return; 
        }
    }

    // Comprehensive built-in database (Common vendors)
    addOuiRange({"00:00:0C", "00:01:42", "00:01:63", "00:01:96", "00:01:C7", "00:02:16", "00:02:4A", "00:02:7D", "00:02:B9", "00:02:FC", "00:03:31", "00:03:6B", "00:03:9F", "00:03:E3", "00:03:FD", "00:04:27", "00:04:4D", "00:04:6D", "00:04:9A", "00:04:C0", "00:04:DD"}, "Cisco Systems");
    addOuiRange({"00:03:93", "00:05:02", "00:0A:27", "00:0D:93", "00:10:FA", "00:14:51", "00:16:CB", "00:17:F2", "00:19:E3", "00:1B:63", "00:1C:B3", "00:1D:4F", "00:1E:52", "00:1F:5B", "00:21:E9", "00:22:41", "00:23:12", "00:23:32", "00:23:6C", "00:24:36", "00:25:00", "00:25:4B", "00:25:BC", "00:26:08", "00:26:4A", "00:26:B0", "00:26:BB", "04:0C:CE", "04:15:52", "04:1E:64", "04:26:65", "04:4B:ED", "04:52:F3", "04:E5:36", "08:00:07", "08:66:98", "08:6D:41", "0C:15:39", "0C:30:21", "0C:3E:9F", "0C:4D:E9", "0C:51:01", "0C:74:C2", "0C:77:1A", "0C:8B:FD", "0C:D7:46"}, "Apple");
    addOuiRange({"00:00:F0", "00:02:78", "00:07:AB", "00:0D:E6", "00:0F:73", "00:12:47", "00:12:FB", "00:13:77", "00:15:99", "00:15:B7", "00:16:32", "00:16:6B", "00:17:C9", "00:17:D5", "00:18:AF", "00:19:01", "00:1A:8A", "00:1B:98", "00:1C:D1", "00:1D:6B", "00:1E:7D", "00:1F:CC", "00:21:19", "00:21:D2", "00:23:39", "00:23:D7", "00:24:54", "00:24:91", "00:24:E4", "00:25:67", "00:26:37", "00:26:5D", "04:18:0F", "04:52:C7", "04:FE:A1", "08:08:C2", "08:37:3D", "08:70:45", "08:D4:6A", "08:EC:A9", "08:F4:AB"}, "Samsung");
    addOuiRange({"00:05:C5", "00:0B:09", "00:18:82", "00:1E:10", "00:22:93", "00:25:68", "00:25:9E", "00:46:4B", "00:54:4E", "00:66:4B", "04:25:C5", "04:BD:70", "04:C0:6F", "04:F9:38", "08:19:A6", "08:4F:0A", "08:63:61", "08:7A:4C", "08:E8:4F", "0C:37:DC", "0C:45:BA", "0C:70:4A", "10:1B:54", "10:44:00", "10:47:80", "10:51:72", "10:5B:AD", "10:C6:1F", "14:3E:BF", "14:5F:94", "14:9D:09", "14:B9:68", "18:22:7E", "18:C5:8A", "18:DE:D7", "1C:1D:67", "1C:44:19", "1C:8E:5C", "20:0B:C7", "20:2B:20", "20:F1:7C", "24:09:95", "24:44:27", "24:69:A5", "24:73:A0", "24:7F:3C", "24:9E:AB", "24:AF:4A", "24:DF:F3"}, "Huawei");
    addOuiRange({"00:03:47", "00:04:23", "00:08:C7", "00:0C:F1", "00:0E:0C", "00:11:75", "00:13:02", "00:13:20", "00:13:E8", "00:15:00", "00:16:6F", "00:16:EA", "00:18:DE", "00:19:D1", "00:1A:92", "00:1B:21", "00:1B:77", "00:1C:BF", "00:1D:E0", "00:1E:64", "00:1E:65", "00:1F:3B", "00:1F:3C", "00:21:5D", "00:21:6A", "00:22:FA", "00:22:FB", "00:23:14", "00:23:15", "00:24:D6", "00:24:D7", "00:26:C6", "00:26:C7", "00:27:0E", "00:27:10"}, "Intel");
    addOuiRange({"00:08:74", "00:0A:E4", "00:0D:56", "00:0F:1F", "00:11:43", "00:12:3F", "00:13:72", "00:14:22", "00:15:C5", "00:16:F0", "00:18:8B", "00:19:B9", "00:1A:A0", "00:1C:23", "00:1D:09", "00:1E:4F", "00:1E:C9", "00:21:70", "00:21:9B", "00:22:19", "00:23:AE", "00:24:E8", "00:25:64", "00:26:B9", "00:F1:F3", "10:98:36", "18:03:73", "18:66:DA", "24:B6:57", "34:40:B5"}, "Dell");
    addOuiRange({"00:01:E6", "00:02:A5", "00:08:02", "00:0B:CD", "00:0D:9D", "00:0E:7F", "00:0F:20", "00:10:83", "00:11:0A", "00:11:85", "00:12:79", "00:13:21", "00:14:38", "00:14:C2", "00:15:60", "00:16:35", "00:17:08", "00:18:FE", "00:19:BB", "00:1A:4B", "00:1B:78", "00:1C:C4", "00:1E:0B", "00:1F:29", "00:21:5A", "00:22:64", "00:23:7D", "00:24:81", "00:25:B3", "00:26:55", "08:00:09", "08:2E:5F", "10:1F:74", "10:60:4B", "18:A9:05", "28:80:23", "34:64:A9", "3C:D9:2B", "40:B0:34", "48:0F:CF"}, "HP");
    addOuiRange({"00:03:FF", "00:0D:3A", "00:12:5A", "00:15:5D", "00:17:FA", "00:1D:D8", "00:22:48", "00:25:AE", "00:50:F2", "10:60:4B", "28:18:78", "50:1A:C5", "60:45:BD", "7C:1E:52", "94:F6:65", "BC:83:85", "C0:3F:0E", "DC:B4:C4", "E8:03:9A", "F0:6E:0B"}, "Microsoft");
    addOuiRange({"00:05:69", "00:0C:29", "00:50:56"}, "VMware");
    addOuiRange({"08:00:27"}, "Oracle VirtualBox");
}

std::string lookupVendor(const std::string& macAddress) {
    if (ouiDatabase.empty()) {
        initOuiDatabase();
    }
    
    if (macAddress.length() < 8) return "Unknown";
    
    std::string oui = macAddress.substr(0, 8);
    std::transform(oui.begin(), oui.end(), oui.begin(), ::toupper);
    
    auto it = ouiDatabase.find(oui);
    if (it != ouiDatabase.end()) {
        return it->second;
    }
    
    return "Unknown";
}
