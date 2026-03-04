#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "gui.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <commctrl.h>

#include <iostream>
#include <thread>
#include <atomic>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ws2_32.lib")

const char* CLASS_NAME = "NNetScanWindow";
const char* WINDOW_TITLE = "Network Scanner";

static MainWindow* g_pMainWindow = nullptr;

MainWindow::MainWindow()
    : hwnd_(nullptr)
    , hComboInterface_(nullptr)
    , hBtnScan_(nullptr)
    , hChkArp_(nullptr)
    , hChkIcmp_(nullptr)
    , hListResults_(nullptr)
    , hProgress_(nullptr)
    , hStatus_(nullptr)
    , hInstance_(nullptr)
    , scanning_(false)
{
    config_.useArp = true;
    config_.useIcmp = true;
    config_.portTimeout = 500;
    config_.maxThreads = 50;
}

MainWindow::~MainWindow() {
}

bool MainWindow::create() {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = wndProc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    
    if (!RegisterClassA(&wc)) {
        return false;
    }
    
    hwnd_ = CreateWindowA(
        CLASS_NAME, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 600,
        nullptr, nullptr, GetModuleHandleA(nullptr), this
    );
    
    if (!hwnd_) {
        return false;
    }
    
    return true;
}

void MainWindow::createControls(HWND parent) {
    HWND hLabel1 = CreateWindowA("STATIC", "Network Interface:",
        WS_CHILD | WS_VISIBLE, 20, 20, 120, 20, parent, nullptr, nullptr, nullptr);
    
    hComboInterface_ = CreateWindowA("COMBOBOX", "",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        140, 18, 500, 300, parent, nullptr, nullptr, nullptr);
    
    HWND hLabelSubnet = CreateWindowA("STATIC", "Subnet (CIDR):",
        WS_CHILD | WS_VISIBLE, 20, 55, 120, 20, parent, nullptr, nullptr, nullptr);
    
    hEditSubnet_ = CreateWindowA("EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
        140, 53, 200, 20, parent, nullptr, nullptr, nullptr);
    
    HWND hLabel2 = CreateWindowA("STATIC", "Scan Methods:",
        WS_CHILD | WS_VISIBLE, 20, 90, 120, 20, parent, nullptr, nullptr, nullptr);
    
    hChkArp_ = CreateWindowA("BUTTON", "ARP Scan",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 140, 90, 100, 20, parent, nullptr, nullptr, nullptr);
    SendMessageA(hChkArp_, BM_SETCHECK, BST_CHECKED, 0);
    
    hChkIcmp_ = CreateWindowA("BUTTON", "ICMP Ping",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 250, 90, 100, 20, parent, nullptr, nullptr, nullptr);
    SendMessageA(hChkIcmp_, BM_SETCHECK, BST_CHECKED, 0);
    
    hBtnScan_ = CreateWindowA("BUTTON", "Start Scan",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        670, 18, 100, 28, parent, nullptr, nullptr, nullptr);
    
    hProgress_ = CreateWindowA(PROGRESS_CLASS, "",
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        20, 120, 750, 20, parent, nullptr, nullptr, nullptr);
    
    hListResults_ = CreateWindowExA(
        0, WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
        20, 150, 750, 365, parent, nullptr, nullptr, nullptr);
    
    LVCOLUMNA col;
    col.mask = LVCF_TEXT | LVCF_WIDTH;
    
    col.pszText = (char*)"IP Address";
    col.cx = 130;
    ListView_InsertColumn(hListResults_, 0, &col);
    
    col.pszText = (char*)"MAC Address";
    col.cx = 140;
    ListView_InsertColumn(hListResults_, 1, &col);
    
    col.pszText = (char*)"Vendor";
    col.cx = 180;
    ListView_InsertColumn(hListResults_, 2, &col);
    
    col.pszText = (char*)"Type";
    col.cx = 80;
    ListView_InsertColumn(hListResults_, 3, &col);
    
    col.pszText = (char*)"Open Ports";
    col.cx = 220;
    ListView_InsertColumn(hListResults_, 4, &col);
    
    hStatus_ = CreateWindowA("STATIC", "Ready",
        WS_CHILD | WS_VISIBLE, 20, 520, 750, 20, parent, nullptr, nullptr, nullptr);
}

void MainWindow::populateInterfaces() {
    interfaces_ = scanner_->getInterfaces();
    
    for (size_t i = 0; i < interfaces_.size(); ++i) {
        std::string display = interfaces_[i].ipAddress + " - " + interfaces_[i].description;
        SendMessageA(hComboInterface_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(display.c_str()));
    }
    
    if (!interfaces_.empty()) {
        SendMessageA(hComboInterface_, CB_SETCURSEL, 0, 0);
        onInterfaceChange();
    }
}

static int maskToCidr(const std::string& mask) {
    uint32_t m = ntohl(inet_addr(mask.c_str()));
    int cidr = 0;
    while (m & 0x80000000) {
        cidr++;
        m <<= 1;
    }
    return cidr;
}

void MainWindow::onInterfaceChange() {
    int sel = SendMessageA(hComboInterface_, CB_GETCURSEL, 0, 0);
    if (sel >= 0 && sel < static_cast<int>(interfaces_.size())) {
        const auto& iface = interfaces_[sel];
        int cidr = maskToCidr(iface.subnetMask);
        
        // Calculate network address
        uint32_t ip = ntohl(inet_addr(iface.ipAddress.c_str()));
        uint32_t mask = (cidr == 0) ? 0 : (~0U << (32 - cidr));
        uint32_t network = ip & mask;
        
        struct in_addr addr;
        addr.S_un.S_addr = htonl(network);
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr, ipStr, INET_ADDRSTRLEN);
        
        std::string subnet = std::string(ipStr) + "/" + std::to_string(cidr);
        SendMessageA(hEditSubnet_, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(subnet.c_str()));
    }
}

void MainWindow::run() {
    MSG msg = {};
    while (GetMessageA(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

LRESULT CALLBACK MainWindow::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CREATE) {
        CREATESTRUCTA* pcs = reinterpret_cast<CREATESTRUCTA*>(lParam);
        g_pMainWindow = static_cast<MainWindow*>(pcs->lpCreateParams);
        g_pMainWindow->hwnd_ = hwnd;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(g_pMainWindow));
        g_pMainWindow->createControls(hwnd);
        g_pMainWindow->scanner_ = std::make_unique<NetworkScanner>();
        g_pMainWindow->populateInterfaces();
        return 0;
    }
    
    MainWindow* pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    if (pThis) {
        return pThis->handleMessage(hwnd, msg, wParam, lParam);
    }
    
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

LRESULT MainWindow::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED && reinterpret_cast<HWND>(lParam) == hBtnScan_) {
                if (!scanning_) {
                    startScan();
                }
            } else if (HIWORD(wParam) == CBN_SELCHANGE && reinterpret_cast<HWND>(lParam) == hComboInterface_) {
                onInterfaceChange();
            }
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    
    return 0;
}

void MainWindow::startScan() {
    int sel = SendMessageA(hComboInterface_, CB_GETCURSEL, 0, 0);
    if (sel < 0 || sel >= static_cast<int>(interfaces_.size())) {
        MessageBoxA(hwnd_, "Please select a network interface.", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    config_.useArp = (SendMessageA(hChkArp_, BM_GETCHECK, 0, 0) == BST_CHECKED);
    config_.useIcmp = (SendMessageA(hChkIcmp_, BM_GETCHECK, 0, 0) == BST_CHECKED);
    
    if (!config_.useArp && !config_.useIcmp) {
        MessageBoxA(hwnd_, "Please select at least one scan method.", "Error", MB_OK | MB_ICONERROR);
        return;
    }
    
    scanning_ = true;
    EnableWindow(hBtnScan_, FALSE);
    SendMessageA(hStatus_, WM_SETTEXT, 0, reinterpret_cast<LPARAM>("Scanning..."));
    ListView_DeleteAllItems(hListResults_);
    SendMessageA(hProgress_, PBM_SETPOS, 0, 0);
    
    const NetworkInterface& iface = interfaces_[sel];
    
    char subnetBuf[256];
    GetWindowTextA(hEditSubnet_, subnetBuf, sizeof(subnetBuf));
    std::string customSubnet = subnetBuf;
    
    std::thread([this, iface, customSubnet]() {
        auto results = scanner_->scanNetwork(iface, config_.useArp, config_.useIcmp,
            [this](int current, int total) {
                int pos = static_cast<int>((current * 100.0) / total);
                SendMessageA(hProgress_, PBM_SETPOS, pos, 0);
                char status[256];
                sprintf_s(status, "Scanning... %d/%d hosts", current, total);
                SendMessageA(hStatus_, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(status));
            },
            [this](const HostInfo& host) {
                addResult(host);
            },
            customSubnet);
        
        SendMessageA(hProgress_, PBM_SETPOS, 100, 0);
        SendMessageA(hStatus_, WM_SETTEXT, 0, reinterpret_cast<LPARAM>("Scan complete"));
        
        scanning_ = false;
        EnableWindow(hBtnScan_, TRUE);
    }).detach();
}

void MainWindow::updateResults(const std::vector<HostInfo>& hosts) {
    for (const auto& host : hosts) {
        addResult(host);
    }
}

void MainWindow::addResult(const HostInfo& host) {
    std::lock_guard<std::mutex> lock(uiMutex_);
    
    LVITEMA item = {};
    item.mask = LVIF_TEXT;
    item.iItem = ListView_GetItemCount(hListResults_);
    
    item.pszText = const_cast<char*>(host.ipAddress.c_str());
    item.iSubItem = 0;
    ListView_InsertItem(hListResults_, &item);
    
    item.pszText = const_cast<char*>(host.macAddress.empty() ? "-" : host.macAddress.c_str());
    item.iSubItem = 1;
    ListView_SetItem(hListResults_, &item);
    
    item.pszText = const_cast<char*>(host.vendor.empty() ? "-" : host.vendor.c_str());
    item.iSubItem = 2;
    ListView_SetItem(hListResults_, &item);
    
    std::string type = host.isGateway ? "Gateway" : "Host";
    item.pszText = const_cast<char*>(type.c_str());
    item.iSubItem = 3;
    ListView_SetItem(hListResults_, &item);
    
    std::string ports;
    for (size_t i = 0; i < host.openPorts.size(); ++i) {
        if (i > 0) ports += ", ";
        ports += std::to_string(host.openPorts[i]);
    }
    item.pszText = const_cast<char*>(ports.empty() ? "-" : ports.c_str());
    item.iSubItem = 4;
    ListView_SetItem(hListResults_, &item);
}
