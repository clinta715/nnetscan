#include "network.h"
#include "arp.h"
#include "portscan.h"
#include "oui_lookup.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#ifndef MIB_IF_TYPE_IEEE80211
#define MIB_IF_TYPE_IEEE80211 71
#endif

#include <iostream>
#include <algorithm>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

NetworkScanner::NetworkScanner() : initialized_(false) {
    WSADATA wsaData;
    initialized_ = WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
    initOuiDatabase();
}

NetworkScanner::~NetworkScanner() {
    if (initialized_) {
        WSACleanup();
    }
}

std::vector<NetworkInterface> NetworkScanner::getInterfaces() {
    std::vector<NetworkInterface> result;
    
    ULONG bufferSize = 15000;
    std::vector<uint8_t> buffer(bufferSize);
    PIP_ADAPTER_INFO pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(buffer.data());
    
    DWORD ret = GetAdaptersInfo(pAdapterInfo, &bufferSize);
    
    if (ret == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(bufferSize);
        pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(buffer.data());
        ret = GetAdaptersInfo(pAdapterInfo, &bufferSize);
    }
    
    if (ret != NO_ERROR) {
        return result;
    }
    
    PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
    while (pAdapter) {
        if ((pAdapter->Type == MIB_IF_TYPE_ETHERNET || pAdapter->Type == MIB_IF_TYPE_IEEE80211) && pAdapter->IpAddressList.IpAddress.String[0] != '0') {
            NetworkInterface iface;
            iface.name = pAdapter->AdapterName;
            iface.description = pAdapter->Description;
            iface.ipAddress = pAdapter->IpAddressList.IpAddress.String;
            iface.subnetMask = pAdapter->IpAddressList.IpMask.String;
            iface.gateway = pAdapter->GatewayList.IpAddress.String;
            
            result.push_back(iface);
        }
        pAdapter = pAdapter->Next;
    }
    
    return result;
}

static uint32_t ipToUint(const std::string& ip) {
    return ntohl(inet_addr(ip.c_str()));
}

static uint32_t calculateBroadcast(uint32_t ip, uint32_t mask) {
    return (ip & mask) | (~mask);
}

static uint32_t calculateNetwork(uint32_t ip, uint32_t mask) {
    return ip & mask;
}

bool pingHost(const std::string& ip, int timeoutMs) {
    HANDLE hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    char sendData[32] = "NNetScan";
    char replyBuffer[sizeof(ICMP_ECHO_REPLY) + 32];
    
    DWORD replySize = sizeof(replyBuffer);
    DWORD timeout = timeoutMs;
    
    uint32_t ipAddr = inet_addr(ip.c_str());
    
    DWORD result = IcmpSendEcho(hIcmpFile, ipAddr, sendData, sizeof(sendData),
                                 nullptr, replyBuffer, replySize, timeout);
    
    IcmpCloseHandle(hIcmpFile);
    
    return result > 0;
}

static std::string lookupHostname(const std::string& ip) {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(ip.c_str());
    
    char hostname[NI_MAXHOST];
    if (getnameinfo((struct sockaddr*)&sa, sizeof(sa), hostname, NI_MAXHOST, nullptr, 0, NI_NAMEREQD) == 0) {
        return hostname;
    }
    return "";
}

void NetworkScanner::stopScan() {
    cancelScan_ = true;
}

std::vector<HostInfo> NetworkScanner::scanNetwork(const NetworkInterface& iface, 
                                                     bool useArp, bool useIcmp,
                                                     std::function<void(int, int)> progressCallback,
                                                     std::function<void(const HostInfo&)> hostCallback,
                                                     const std::string& customSubnet) {
    cancelScan_ = false;
    std::vector<HostInfo> results;
    
    uint32_t network, mask, broadcast;
    
    if (!customSubnet.empty()) {
        size_t slashPos = customSubnet.find('/');
        if (slashPos != std::string::npos) {
            std::string ipPart = customSubnet.substr(0, slashPos);
            int cidr = std::stoi(customSubnet.substr(slashPos + 1));
            
            uint32_t baseIp = ntohl(inet_addr(ipPart.c_str()));
            mask = (cidr == 0) ? 0 : (~0U << (32 - cidr));
            network = baseIp & mask;
            broadcast = network | (~mask);
        } else {
            // Fallback to interface if parsing fails or just an IP
            uint32_t ip = ipToUint(iface.ipAddress);
            mask = ipToUint(iface.subnetMask);
            network = calculateNetwork(ip, mask);
            broadcast = calculateBroadcast(ip, mask);
        }
    } else {
        uint32_t ip = ipToUint(iface.ipAddress);
        mask = ipToUint(iface.subnetMask);
        network = calculateNetwork(ip, mask);
        broadcast = calculateBroadcast(ip, mask);
    }
    
    uint32_t totalHosts = broadcast - network - 1;
    // Limit to /24 by default if too large for now, or just let it run
    // For safety in this tool, let's limit to 1024 hosts for now to avoid freezing
    if (totalHosts > 1024) totalHosts = 1024;
    
    uint32_t selfIp = ipToUint(iface.ipAddress);
    
    std::atomic<int> scanned{0};
    std::mutex resultsMutex;
    
    std::vector<int> commonPorts = getCommonPorts();
    
    std::vector<std::thread> threads;
    int threadCount = (totalHosts < 32) ? static_cast<int>(totalHosts) : 32;
    int hostsPerThread = (totalHosts + threadCount - 1) / threadCount;
    
    for (int t = 0; t < threadCount; ++t) {
        threads.emplace_back([&, t, hostsPerThread, broadcast, network, selfIp]() {
            uint32_t startHost = network + 1 + t * hostsPerThread;
            uint32_t endHost = startHost + hostsPerThread;
            if (endHost > broadcast) endHost = broadcast;
            
            for (uint32_t hostIp = startHost; hostIp < endHost; ++hostIp) {
                if (cancelScan_) break;
                if (hostIp == selfIp) continue;
                
                HostInfo info;
                info.ipAddress = ipToString(htonl(hostIp));
                info.isReachable = false;
                info.isGateway = (hostIp == ipToUint(iface.gateway));
                
                std::string mac;
                bool arpSuccess = false;
                
                if (useArp) {
                    arpSuccess = sendArpRequest(info.ipAddress, iface.ipAddress, mac);
                    if (!arpSuccess) {
                        auto arpTable = getArpTable();
                        for (const auto& entry : arpTable) {
                            if (entry.ipAddress == info.ipAddress) {
                                mac = entry.macAddress;
                                arpSuccess = true;
                                break;
                            }
                        }
                    }
                    if (arpSuccess) {
                        info.macAddress = mac;
                        info.vendor = lookupVendor(mac);
                        info.isReachable = true;
                    }
                }
                
                if (!info.isReachable && useIcmp) {
                    info.isReachable = pingHost(info.ipAddress, 200);
                    if (info.isReachable && info.macAddress.empty() && useArp) {
                        bool gotMac = sendArpRequest(info.ipAddress, iface.ipAddress, mac);
                        if (!gotMac) {
                            auto arpTable = getArpTable();
                            for (const auto& entry : arpTable) {
                                if (entry.ipAddress == info.ipAddress) {
                                    mac = entry.macAddress;
                                    gotMac = true;
                                    break;
                                }
                            }
                        }
                        if (gotMac) {
                            info.macAddress = mac;
                            info.vendor = lookupVendor(mac);
                        }
                    }
                }
                
                if (info.isReachable) {
                    info.hostname = lookupHostname(info.ipAddress);
                    info.openPorts = scanPortsConcurrent(info.ipAddress, commonPorts, 300, 20);
                    
                    {
                        std::lock_guard<std::mutex> lock(resultsMutex);
                        results.push_back(info);
                    }
                    
                    if (hostCallback && !cancelScan_) {
                        hostCallback(info);
                    }
                }
                
                int current = ++scanned;
                if (progressCallback && !cancelScan_) {
                    progressCallback(current, totalHosts);
                }
            }
        });
    }
    
    for (auto& th : threads) {
        th.join();
    }
    
    return results;
}

std::vector<int> NetworkScanner::scanPorts(const std::string& ip, const std::vector<int>& ports) {
    return scanPortsConcurrent(ip, ports, 500, 20);
}
