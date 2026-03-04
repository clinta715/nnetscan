#include "arp.h"

#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>

#include <vector>
#include <cstring>
#include <cstdio>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

std::string macToString(const uint8_t* mac) {
    char buffer[18];
    sprintf_s(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", 
              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(buffer);
}

bool stringToIp(const std::string& str, uint32_t& ipOut) {
    ipOut = inet_addr(str.c_str());
    return ipOut != INADDR_NONE;
}

std::string ipToString(uint32_t ip) {
    struct in_addr addr;
    addr.s_addr = ip;
    return std::string(inet_ntoa(addr));
}

std::vector<ArpEntry> getArpTable() {
    std::vector<ArpEntry> entries;
    
    ULONG bufferSize = 15000;
    std::vector<uint8_t> buffer(bufferSize);
    PMIB_IPNETTABLE pArpTable = reinterpret_cast<PMIB_IPNETTABLE>(buffer.data());
    
    DWORD ret = GetIpNetTable(pArpTable, &bufferSize, FALSE);
    
    if (ret == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(bufferSize);
        pArpTable = reinterpret_cast<PMIB_IPNETTABLE>(buffer.data());
        ret = GetIpNetTable(pArpTable, &bufferSize, FALSE);
    }
    
    if (ret != NO_ERROR) {
        return entries;
    }
    
    for (DWORD i = 0; i < pArpTable->dwNumEntries; ++i) {
        MIB_IPNETROW& row = pArpTable->table[i];
        
        if (row.dwPhysAddrLen == 6 && row.dwAddr != 0) {
            ArpEntry entry;
            entry.ipAddress = ipToString(row.dwAddr);
            entry.macAddress = macToString(row.bPhysAddr);
            entry.ifIndex = row.dwIndex;
            entries.push_back(entry);
        }
    }
    
    return entries;
}

bool sendArpRequest(const std::string& targetIp, const std::string& interfaceIp, std::string& macOut) {
    uint32_t targetAddr = inet_addr(targetIp.c_str());
    uint32_t interfaceAddr = inet_addr(interfaceIp.c_str());
    
    if (targetAddr == INADDR_NONE || interfaceAddr == INADDR_NONE) {
        return false;
    }
    
    uint8_t mac[6] = {0};
    ULONG macLen = sizeof(mac);
    
    DWORD ret = SendARP(targetAddr, interfaceAddr, mac, &macLen);
    
    if (ret == NO_ERROR && macLen == 6) {
        bool isValid = false;
        for (int i = 0; i < 6; i++) {
            if (mac[i] != 0) {
                isValid = true;
                break;
            }
        }
        if (isValid) {
            macOut = macToString(mac);
            return true;
        }
    }
    
    return false;
}
