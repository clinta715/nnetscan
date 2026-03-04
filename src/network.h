#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <functional>

struct NetworkInterface {
    std::string name;
    std::string description;
    std::string ipAddress;
    std::string subnetMask;
    std::string gateway;
    std::vector<std::string> dnsServers;
};

struct HostInfo {
    std::string ipAddress;
    std::string macAddress;
    std::string vendor;
    bool isReachable;
    std::vector<int> openPorts;
    bool isGateway;
};

class NetworkScanner {
public:
    NetworkScanner();
    ~NetworkScanner();

    std::vector<NetworkInterface> getInterfaces();
    std::vector<HostInfo> scanNetwork(const NetworkInterface& iface, bool useArp, bool useIcmp, 
                                      std::function<void(int, int)> progressCallback,
                                      std::function<void(const HostInfo&)> hostCallback = nullptr,
                                      const std::string& customSubnet = "");
    std::vector<int> scanPorts(const std::string& ip, const std::vector<int>& ports);

private:
    bool initialized_;
};
