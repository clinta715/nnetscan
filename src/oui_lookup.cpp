#include "oui_lookup.h"

#include <unordered_map>
#include <string>
#include <cstring>
#include <algorithm>

static std::unordered_map<std::string, std::string> ouiDatabase;

void initOuiDatabase() {
    if (!ouiDatabase.empty()) return;
    
    ouiDatabase = {
        {"00:00:0C", "Cisco Systems"},
        {"00:01:63", "Cisco Systems"},
        {"00:01:96", "Cisco Systems"},
        {"00:01:C7", "Cisco Systems"},
        {"00:01:E7", "Cisco Systems"},
        {"00:02:16", "Cisco Systems"},
        {"00:02:4A", "Cisco Systems"},
        {"00:02:7D", "Cisco Systems"},
        {"00:02:B9", "Cisco Systems"},
        {"00:02:FC", "Cisco Systems"},
        {"00:03:31", "Cisco Systems"},
        {"00:03:6B", "Cisco Systems"},
        {"00:03:9F", "Cisco Systems"},
        {"00:03:E3", "Cisco Systems"},
        {"00:03:FD", "Cisco Systems"},
        {"00:04:27", "Cisco Systems"},
        {"00:04:4D", "Cisco Systems"},
        {"00:04:6D", "Cisco Systems"},
        {"00:04:9A", "Cisco Systems"},
        {"00:04:C0", "Cisco Systems"},
        {"00:04:DD", "Cisco Systems"},
        
        {"00:0C:29", "VMware"},
        {"00:50:56", "VMware"},
        
        {"00:15:5D", "Microsoft Hyper-V"},
        {"00:03:FF", "Microsoft"},
        {"00:0D:3A", "Microsoft"},
        {"00:12:5A", "Microsoft"},
        {"00:15:5D", "Microsoft"},
        {"00:17:FA", "Microsoft"},
        {"00:1D:D8", "Microsoft"},
        {"00:22:48", "Microsoft"},
        {"00:25:AE", "Microsoft"},
        
        {"00:1A:A0", "Dell"},
        {"00:14:22", "Dell"},
        {"00:18:8B", "Dell"},
        {"00:19:B9", "Dell"},
        {"00:1C:23", "Dell"},
        {"00:1D:09", "Dell"},
        {"00:1E:4F", "Dell"},
        {"00:1E:C9", "Dell"},
        
        {"00:16:3E", "Xen"},
        {"00:18:60", "Xen"},
        
        {"00:50:B6", "Linksys"},
        {"00:1E:E5", "Linksys"},
        {"00:1F:33", "Linksys"},
        {"00:23:69", "Cisco-Linksys"},
        
        {"00:14:BF", "Cisco-Linksys"},
        {"00:1A:70", "Cisco-Linksys"},
        
        {"00:24:B2", "Netgear"},
        {"00:26:F2", "Netgear"},
        {"00:1B:2F", "Netgear"},
        {"00:1E:2A", "Netgear"},
        {"00:1F:33", "Netgear"},
        
        {"00:14:6C", "Netgear"},
        {"00:18:4D", "Netgear"},
        
        {"00:26:4A", "TP-Link"},
        {"00:27:19", "TP-Link"},
        {"00:50:43", "TP-Link"},
        {"00:E0:4C", "TP-Link"},
        
        {"00:1D:0F", "TP-Link"},
        {"00:21:27", "TP-Link"},
        {"00:23:CD", "TP-Link"},
        
        {"A4:2C:08", "TP-Link"},
        {"A4:B1:97", "TP-Link"},
        {"B0:4E:26", "TP-Link"},
        {"B0:95:75", "TP-Link"},
        
        {"34:E8:94", "TP-Link"},
        {"50:C7:BF", "TP-Link"},
        
        {"00:1F:33", "Netgear"},
        {"00:22:6B", "Netgear"},
        
        {"00:25:9C", "Cisco-Linksys"},
        
        {"00:1A:A1", "Giga-Byte"},
        {"00:1D:60", "Giga-Byte"},
        {"00:1E:67", "Giga-Byte"},
        {"00:1F:D0", "Giga-Byte"},
        
        {"00:18:F3", "IBM"},
        {"00:19:41", "IBM"},
        {"00:1A:64", "IBM"},
        {"00:1B:78", "IBM"},
        {"00:1C:0C", "IBM"},
        {"00:1D:09", "IBM"},
        {"00:1E:6F", "IBM"},
        
        {"00:16:3F", "Intel"},
        {"00:17:35", "Intel"},
        {"00:18:DE", "Intel"},
        {"00:19:D1", "Intel"},
        {"00:1A:92", "Intel"},
        {"00:1B:21", "Intel"},
        {"00:1B:77", "Intel"},
        {"00:1C:BF", "Intel"},
        {"00:1D:E0", "Intel"},
        {"00:1E:64", "Intel"},
        {"00:1E:67", "Intel"},
        {"00:1F:3B", "Intel"},
        {"00:20:E0", "Intel"},
        
        {"00:24:D6", "Hewlett-Packard"},
        {"00:26:55", "Hewlett-Packard"},
        {"00:27:0D", "Hewlett-Packard"},
        {"00:30:C1", "Hewlett-Packard"},
        {"00:40:17", "Hewlett-Packard"},
        {"00:50:8B", "Hewlett-Packard"},
        {"00:60:B0", "Hewlett-Packard"},
        {"00:80:A0", "Hewlett-Packard"},
        {"08:2E:5F", "Hewlett-Packard"},
        {"10:1F:74", "Hewlett-Packard"},
        
        {"00:00:63", "Racal-Datacom"},
        {"00:00:85", "Racal-Datacom"},
        {"00:01:03", "Racal-Datacom"},
        {"00:02:5D", "Racal-Datacom"},
        
        {"00:00:6B", "Artemis"},
        {"00:00:8F", "Hayes"},
        {"00:00:AA", "Western Digital"},
        {"00:00:BB", "Datapoint"},
        
        {"00:00:00", "XEROX"},
        {"00:01:00", "XEROX"},
        {"00:02:00", "XEROX"},
        {"00:03:E3", "XEROX"},
        
        {"00:80:37", "Britton Lee"},
        {"00:00:56", "Satelcom"},
        {"00:00:96", "Arc"},
        {"00:00:9A", "Reliance Computer"},
        {"00:00:CD", "Nomics"},
        
        {"00:80:AD", "Autophon"},
        {"00:00:D7", "Telco Systems"},
        {"00:00:E1", "webel"},
        {"00:00:E8", "Savid"},
        {"00:00:F3", "Toshiba"},
        {"00:01:04", "Telco Systems"},
        {"00:01:08", "Retix"},
        {"00:01:1B", "Apple"},
        {"00:01:1C", "Madge"},
        {"00:01:1E", "HCL"},
        {"00:01:24", "Adra"},
        {"00:01:2E", "Mii"},
        
        {"00:0D:56", "Dell"},
        {"00:11:11", "Dell"},
        {"00:13:72", "Dell"},
        {"00:14:22", "Dell"},
        {"00:15:C5", "Dell"},
        {"00:16:F0", "Dell"},
        {"00:18:8B", "Dell"},
        {"00:19:B9", "Dell"},
        
        {"00:1A:A0", "Dell"},
        {"00:1C:23", "Dell"},
        
        {"00:1E:4F", "Dell"},
        {"00:1E:C9", "Dell"},
        {"00:21:70", "Dell"},
        {"00:21:9B", "Dell"},
        {"00:22:19", "Dell"},
        {"00:23:AE", "Dell"},
        
        {"08:00:20", "Oracle"},
        {"00:03:BA", "Oracle"},
        
        {"00:0B:CD", "Juniper Networks"},
        {"00:0C:F6", "Juniper Networks"},
        {"00:0E:84", "Juniper Networks"},
        {"00:0F:20", "Juniper Networks"},
        {"00:0F:CF", "Juniper Networks"},
        {"00:10:DB", "Juniper Networks"},
        
        {"00:17:CB", "Panasonic"},
        {"00:17:C5", "Panasonic"},
        {"00:1E:C5", "Panasonic"},
        
        {"00:1A:0B", "Philips"},
        {"00:1B:1B", "Philips"},
        
        {"00:1B:EA", "LG Electronics"},
        {"00:1E:A1", "LG Electronics"},
        
        {"00:0A:27", "D-Link"},
        {"00:0A:3B", "D-Link"},
        {"00:0B:DB", "D-Link"},
        {"00:0D:56", "D-Link"},
        {"00:0E:A2", "D-Link"},
        {"00:0F:3D", "D-Link"},
        {"00:11:95", "D-Link"},
        {"00:13:46", "D-Link"},
        {"00:15:E9", "D-Link"},
        {"00:17:9A", "D-Link"},
        {"00:19:5B", "D-Link"},
        {"00:1B:11", "D-Link"},
        {"00:1C:F0", "D-Link"},
        {"00:1E:58", "D-Link"},
        {"00:21:91", "D-Link"},
        {"00:22:B0", "D-Link"},
        {"00:24:01", "D-Link"},
        
        {"00:26:5A", "D-Link"},
        {"00:50:BA", "D-Link"},
        
        {"00:0E:35", "Cameo"},
        {"00:0E:A5", "Cameo"},
        {"00:12:3F", "Cameo"},
        {"00:14:6C", "Cameo"},
        
        {"00:0E:D6", "Allied Telesyn"},
        
        {"00:1E:73", "Matsushita"},
        
        {"00:24:36", "Trendnet"},
        
        {"00:14:D1", "Trendnet"},
        
        {"00:1D:7E", "Trendnet"},
        
        {"00:0C:41", "Trendnet"},
        
        {"00:19:DB", "Netgear"},
        
        {"00:1F:33", "Netgear"},
        
        {"00:1A:2B", "Netgear"},
        
        {"00:1B:2F", "Netgear"},
        
        {"00:22:6B", "Netgear"},
        
        {"00:24:B2", "Netgear"},
        
        {"00:26:F2", "Netgear"},
        
        {"28:C6:8E", "Netgear"},
        
        {"2C:B0:5D", "Netgear"},
        
        {"44:94:FC", "Netgear"},
        
        {"6C:B0:CE", "Netgear"},
        
        {"A0:04:60", "Netgear"},
        
        {"A0:21:B7", "Netgear"},
        
        {"A0:40:A0", "Netgear"},
        
        {"AC:84:C6", "Netgear"},
        
        {"C0:3F:0E", "Netgear"},
        
        {"C4:04:15", "Netgear"},
        
        {"C4:3D:C7", "Netgear"},
        
        {"C8:9E:43", "Netgear"},
        
        {"00:1F:33", "Netgear"},
        
        {"00:26:F2", "Netgear"},
        
        {"28:10:7B", "ASUSTek"},
        
        {"00:1D:60", "ASUSTek"},
        
        {"00:1E:8C", "ASUSTek"},
        
        {"00:1F:C6", "ASUSTek"},
        
        {"00:22:15", "ASUSTek"},
        
        {"00:23:54", "ASUSTek"},
        
        {"00:24:8C", "ASUSTek"},
        
        {"00:26:18", "ASUSTek"},
        
        {"00:27:0B", "ASUSTek"},
        
        {"08:60:6E", "ASUSTek"},
        
        {"10:BF:48", "ASUSTek"},
        
        {"14:DA:E9", "ASUSTek"},
        
        {"14:DD:A9", "ASUSTek"},
        
        {"1C:87:2C", "ASUSTek"},
        
        {"1C:B7:2C", "ASUSTek"},
        
        {"20:CF:30", "ASUSTek"},
        
        {"2C:4D:54", "ASUSTek"},
        
        {"2C:56:DC", "ASUSTek"},
        
        {"30:5A:3A", "ASUSTek"},
        
        {"30:85:A9", "ASUSTek"},
        
        {"38:D5:47", "ASUSTek"},
        
        {"3C:97:0E", "ASUSTek"},
        
        {"40:16:7E", "ASUSTek"},
        
        {"48:5B:39", "ASUSTek"},
        
        {"4C:ED:FB", "ASUSTek"},
        
        {"50:46:5D", "ASUSTek"},
        
        {"54:04:96", "ASUSTek"},
        
        {"54:A0:50", "ASUSTek"},
        
        {"60:45:CB", "ASUSTek"},
        
        {"60:A4:4C", "ASUSTek"},
        
        {"78:24:AF", "ASUSTek"},
        
        {"88:D7:F6", "ASUSTek"},
        
        {"90:E6:BA", "ASUSTek"},
        
        {"9C:5C:8E", "ASUSTek"},
        
        {"AC:22:0B", "ASUSTek"},
        
        {"AC:9E:17", "ASUSTek"},
        
        {"B0:6E:BF", "ASUSTek"},
        
        {"BC:AE:C5", "ASUSTek"},
        
        {"BC:EE:7B", "ASUSTek"},
        
        {"C8:60:00", "ASUSTek"},
        
        {"F0:79:59", "ASUSTek"},
        
        {"F4:6D:04", "ASUSTek"},
        
        {"F8:32:E4", "ASUSTek"},
    };
}

std::string lookupVendor(const std::string& macAddress) {
    if (ouiDatabase.empty()) {
        initOuiDatabase();
    }
    
    std::string oui = macAddress.substr(0, 8);
    std::transform(oui.begin(), oui.end(), oui.begin(), ::toupper);
    
    auto it = ouiDatabase.find(oui);
    if (it != ouiDatabase.end()) {
        return it->second;
    }
    
    return "Unknown";
}
