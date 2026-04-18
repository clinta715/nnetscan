#include "cli.h"
#include "network.h"
#include "portscan.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <csignal>

static volatile bool g_cliCancel = false;

static void cliSignalHandler(int) {
    g_cliCancel = true;
}

static void printUsage() {
    std::cout <<
        "NNetScan - Network Scanner\n"
        "\n"
        "Usage:\n"
        "  nnetscan                                  Launch GUI mode\n"
        "  nnetscan --list-interfaces                List network interfaces\n"
        "  nnetscan --subnet <CIDR> [options]        Run a scan\n"
        "\n"
        "Options:\n"
        "  --subnet <CIDR>         Subnet to scan in CIDR notation (e.g. 192.168.1.0/24)\n"
        "  --interface <index>     Interface index to use (default: auto-detect)\n"
        "  --arp                   Enable ARP scan (default: on)\n"
        "  --no-arp                Disable ARP scan\n"
        "  --icmp                  Enable ICMP ping (default: on)\n"
        "  --no-icmp               Disable ICMP ping\n"
        "  --json                  Output results as JSON\n"
        "  --list-interfaces       List available network interfaces\n"
        "  --help                  Show this help message\n"
        "\n"
        "Examples:\n"
        "  nnetscan --subnet 192.168.1.0/24\n"
        "  nnetscan --subnet 10.0.0.0/24 --no-icmp\n"
        "  nnetscan --subnet 192.168.1.0/24 --json\n";
}

static void printInterfaces(const std::vector<NetworkInterface>& interfaces) {
    if (interfaces.empty()) {
        std::cout << "No network interfaces found.\n";
        return;
    }

    std::cout << "Available network interfaces:\n\n";
    std::cout << "  " << std::left << std::setw(5) << "Idx"
              << std::setw(18) << "IP Address"
              << std::setw(18) << "Subnet Mask"
              << std::setw(18) << "Gateway"
              << "Description\n";
    std::cout << "  " << std::string(90, '-') << "\n";

    for (size_t i = 0; i < interfaces.size(); ++i) {
        const auto& iface = interfaces[i];
        std::cout << "  " << std::left << std::setw(5) << i
                  << std::setw(18) << iface.ipAddress
                  << std::setw(18) << iface.subnetMask
                  << std::setw(18) << iface.gateway
                  << iface.description << "\n";
    }
}

static std::string formatPorts(const std::vector<int>& ports) {
    std::string result;
    for (size_t i = 0; i < ports.size(); ++i) {
        if (i > 0) result += ", ";
        result += std::to_string(ports[i]);
        const char* service = getPortServiceName(ports[i]);
        if (strcmp(service, "Unknown") != 0) {
            result += " (";
            result += service;
            result += ")";
        }
    }
    return result;
}

static void printResults(const std::vector<HostInfo>& hosts) {
    if (hosts.empty()) {
        std::cout << "\nNo hosts found.\n";
        return;
    }

    std::cout << "\nScan complete. " << hosts.size() << " host(s) found:\n\n";

    int ipW = 16, hostW = 20, macW = 18, vendorW = 22, typeW = 9;

    std::cout << "  " << std::left << std::setw(ipW) << "IP Address"
              << std::setw(hostW) << "Hostname"
              << std::setw(macW) << "MAC Address"
              << std::setw(vendorW) << "Vendor"
              << std::setw(typeW) << "Type"
              << "Open Ports\n";
    std::cout << "  " << std::string(120, '-') << "\n";

    for (const auto& host : hosts) {
        std::cout << "  " << std::left << std::setw(ipW) << host.ipAddress
                  << std::setw(hostW) << (host.hostname.empty() ? "-" : host.hostname)
                  << std::setw(macW) << (host.macAddress.empty() ? "-" : host.macAddress)
                  << std::setw(vendorW) << (host.vendor.empty() ? "-" : host.vendor)
                  << std::setw(typeW) << (host.isGateway ? "Gateway" : "Host")
                  << (host.openPorts.empty() ? "-" : formatPorts(host.openPorts))
                  << "\n";
    }
}

static void printJson(const std::vector<HostInfo>& hosts) {
    std::cout << "[\n";
    for (size_t i = 0; i < hosts.size(); ++i) {
        const auto& h = hosts[i];
        std::cout << "  {\n";
        std::cout << "    \"ipAddress\": \"" << h.ipAddress << "\",\n";
        std::cout << "    \"hostname\": \"" << (h.hostname.empty() ? "" : h.hostname) << "\",\n";
        std::cout << "    \"macAddress\": \"" << (h.macAddress.empty() ? "" : h.macAddress) << "\",\n";
        std::cout << "    \"vendor\": \"" << (h.vendor.empty() ? "" : h.vendor) << "\",\n";
        std::cout << "    \"isGateway\": " << (h.isGateway ? "true" : "false") << ",\n";
        std::cout << "    \"openPorts\": [";
        for (size_t j = 0; j < h.openPorts.size(); ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << h.openPorts[j];
        }
        std::cout << "]\n";
        std::cout << "  }" << (i + 1 < hosts.size() ? "," : "") << "\n";
    }
    std::cout << "]\n";
}

int runCli(int argc, char* argv[]) {
    bool listInterfaces = false;
    bool useArp = true;
    bool useIcmp = true;
    bool jsonOutput = false;
    std::string subnet;
    int ifaceIndex = -1;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else if (arg == "--list-interfaces") {
            listInterfaces = true;
        } else if (arg == "--subnet") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --subnet requires a CIDR argument (e.g. 192.168.1.0/24)\n";
                return 1;
            }
            subnet = argv[++i];
        } else if (arg == "--interface") {
            if (i + 1 >= argc) {
                std::cerr << "Error: --interface requires an index argument\n";
                return 1;
            }
            ifaceIndex = std::stoi(argv[++i]);
        } else if (arg == "--arp") {
            useArp = true;
        } else if (arg == "--no-arp") {
            useArp = false;
        } else if (arg == "--icmp") {
            useIcmp = true;
        } else if (arg == "--no-icmp") {
            useIcmp = false;
        } else if (arg == "--json") {
            jsonOutput = true;
        } else {
            std::cerr << "Error: Unknown option '" << arg << "'\n";
            std::cerr << "Use --help for usage information.\n";
            return 1;
        }
    }

    if (!useArp && !useIcmp && !listInterfaces) {
        std::cerr << "Error: At least one scan method (ARP or ICMP) must be enabled.\n";
        return 1;
    }

    NetworkScanner scanner;
    auto interfaces = scanner.getInterfaces();

    if (listInterfaces) {
        printInterfaces(interfaces);
        return 0;
    }

    if (subnet.empty()) {
        std::cerr << "Error: --subnet is required for CLI scanning.\n";
        std::cerr << "Use --list-interfaces to see available interfaces.\n";
        return 1;
    }

    NetworkInterface iface;
    if (ifaceIndex >= 0) {
        if (ifaceIndex >= static_cast<int>(interfaces.size())) {
            std::cerr << "Error: Interface index " << ifaceIndex << " is out of range.\n";
            std::cerr << "Use --list-interfaces to see available interfaces.\n";
            return 1;
        }
        iface = interfaces[ifaceIndex];
    } else {
        bool found = false;
        for (const auto& ifc : interfaces) {
            size_t slashPos = subnet.find('/');
            if (slashPos != std::string::npos) {
                std::string subnetBase = subnet.substr(0, slashPos);
                if (ifc.ipAddress.substr(0, subnetBase.rfind('.')) ==
                    subnetBase.substr(0, subnetBase.rfind('.'))) {
                    iface = ifc;
                    found = true;
                    break;
                }
            }
        }
        if (!found && !interfaces.empty()) {
            iface = interfaces[0];
        } else if (interfaces.empty()) {
            std::cerr << "Error: No suitable network interface found.\n";
            return 1;
        }
    }

    std::cout << "Scanning " << subnet << " (via " << iface.ipAddress << ")...\n";

    signal(SIGINT, cliSignalHandler);

    auto results = scanner.scanNetwork(iface, useArp, useIcmp,
        [](int current, int total) {
            int pct = static_cast<int>((current * 100.0) / total);
            std::cout << "\r  Progress: " << current << "/" << total
                      << " (" << pct << "%)" << std::flush;
        },
        nullptr,
        subnet);

    if (g_cliCancel) {
        std::cout << "\n\nScan interrupted by user.\n";
    }

    if (jsonOutput) {
        printJson(results);
    } else {
        printResults(results);
    }

    return 0;
}
