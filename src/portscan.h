#pragma once

#include <string>
#include <vector>
#include <cstdint>

std::vector<int> getCommonPorts();
bool scanPort(const std::string& ip, int port, int timeoutMs = 500);
std::vector<int> scanPortsConcurrent(const std::string& ip, const std::vector<int>& ports, int timeoutMs = 500, int maxThreads = 50);
const char* getPortServiceName(int port);
