#include "portscan.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

std::vector<int> getCommonPorts() {
    return {
        21,   22,   23,   25,   53,   80,   110,  111,  135,  139,
        143,  443,  445,  993,  995,  1723, 3306, 3389, 5900, 8080,
        8443
    };
}

bool scanPort(const std::string& ip, int port, int timeoutMs) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return false;
    }
    
    DWORD timeout = timeoutMs;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
    
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(port));
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    int result = connect(sock, (SOCKADDR*)&addr, sizeof(addr));
    closesocket(sock);
    
    return result == 0;
}

static void scanPortThread(const std::string& ip, int port, int timeoutMs, std::vector<int>& results, std::mutex& resultsMutex) {
    if (scanPort(ip, port, timeoutMs)) {
        std::lock_guard<std::mutex> lock(resultsMutex);
        results.push_back(port);
    }
}

std::vector<int> scanPortsConcurrent(const std::string& ip, const std::vector<int>& ports, 
                                      int timeoutMs, int maxThreads) {
    std::vector<int> openPortsResult;
    std::mutex portMutex;
    
    std::vector<std::thread> threads;
    
    for (int port : ports) {
        if (threads.size() >= static_cast<size_t>(maxThreads)) {
            for (auto it = threads.begin(); it != threads.end();) {
                if (it->joinable()) {
                    it->join();
                    it = threads.erase(it);
                } else {
                    ++it;
                }
            }
        }
        
        threads.emplace_back(scanPortThread, ip, port, timeoutMs, std::ref(openPortsResult), std::ref(portMutex));
    }
    
    for (auto& th : threads) {
        if (th.joinable()) th.join();
    }
    
    std::sort(openPortsResult.begin(), openPortsResult.end());
    return openPortsResult;
}

const char* getPortServiceName(int port) {
    switch (port) {
        case 21:   return "FTP";
        case 22:   return "SSH";
        case 23:   return "Telnet";
        case 25:   return "SMTP";
        case 53:   return "DNS";
        case 80:   return "HTTP";
        case 110:  return "POP3";
        case 111:  return "RPC";
        case 135:  return "MSRPC";
        case 139:  return "NetBIOS";
        case 143:  return "IMAP";
        case 443:  return "HTTPS";
        case 445:  return "SMB";
        case 993:  return "IMAPS";
        case 995:  return "POP3S";
        case 1723: return "PPTP";
        case 3306: return "MySQL";
        case 3389: return "RDP";
        case 5900: return "VNC";
        case 8080: return "HTTP-ALT";
        case 8443: return "HTTPS-ALT";
        default:   return "Unknown";
    }
}
