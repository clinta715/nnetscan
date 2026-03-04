#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct ArpEntry {
    std::string ipAddress;
    std::string macAddress;
    uint32_t ifIndex;
};

std::vector<ArpEntry> getArpTable();
bool sendArpRequest(const std::string& targetIp, const std::string& interfaceIp, std::string& macOut);
std::string macToString(const uint8_t* mac);
bool stringToIp(const std::string& str, uint32_t& ipOut);
std::string ipToString(uint32_t ip);
