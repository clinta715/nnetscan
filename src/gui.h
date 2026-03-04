#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <windows.h>
#include <mutex>

#include "network.h"

struct ScanConfig {
    bool useArp;
    bool useIcmp;
    int portTimeout;
    int maxThreads;
};

class NetworkScanner;

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool create();
    void run();

private:
    static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void createControls(HWND parent);
    void populateInterfaces();
    void onInterfaceChange();
    void startScan();
    void updateResults(const std::vector<HostInfo>& hosts);
    void addResult(const HostInfo& host);
    void onScanProgress(int current, int total);

    HWND hwnd_;
    HWND hComboInterface_;
    HWND hEditSubnet_;
    HWND hBtnScan_;
    HWND hChkArp_;
    HWND hChkIcmp_;
    HWND hListResults_;
    HWND hProgress_;
    HWND hStatus_;

    std::mutex uiMutex_;
    std::vector<NetworkInterface> interfaces_;
    std::unique_ptr<NetworkScanner> scanner_;
    ScanConfig config_;

    HINSTANCE hInstance_;
    bool scanning_;
};
