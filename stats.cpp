#include "Stats.h"

#include <iostream>
#include <string>
#include <windows.h>
#include <intrin.h>
#include <conio.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iomanip> 
#include <limits>

#ifdef _MSC_VER
#pragma comment(lib, "pdh.lib")
#endif

// Constructor
stats::stats(){
    //CPU
    this->CPUFrequency = 0.0;
    this->CPUUtilization = 0.0;
    //RAM
    this->RAMTotal = 0.0;
    this->RAMUsed = 0.0;
    this->RAMUtilization = 0.0;
    //DISK
    this->DISKTotal = 0.0;
    this->DISKUsed = 0.0;
    this->DISKUtilization = 0.0;
    // NETWORK
    this->WiFiSend = 0.0;
    this->WiFiReceive = 0.0;
    this->EthernetSend = 0.0;
    this->EthernetReceive = 0.0;
    this->NETWORKSourceSend = "None";
    this->NETWORKSourceReceive = "None";
};

// Destructor
stats::~stats(){};

//Cpu - Frequency
double stats::GetCPUFrequency(){
    LARGE_INTEGER frequency, start, end;
    DWORD64 startTime, endTime;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    startTime = __rdtsc();

    Sleep(1000);

    endTime = __rdtsc();
    QueryPerformanceCounter(&end);

    double timeDiff = static_cast<double>(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    CPUFrequency = static_cast<double>(endTime - startTime) / (timeDiff * 1000000); 

    if (CPUFrequency < 0) {CPUFrequency = 0.0;}

    return CPUFrequency;
};


//Cpu - Utilization
double stats::GetCPUUtilization(){
    FILETIME idleTime, kernelTime, userTime;
    FILETIME prevIdleTime, prevKernelTime, prevUserTime;

    GetSystemTimes(&prevIdleTime, &prevKernelTime, &prevUserTime);
    Sleep(1000);
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
    
    ULONGLONG idleDiff = (*(ULONGLONG*)&idleTime) - (*(ULONGLONG*)&prevIdleTime);
    ULONGLONG kernelDiff = (*(ULONGLONG*)&kernelTime) - (*(ULONGLONG*)&prevKernelTime);
    ULONGLONG userDiff = (*(ULONGLONG*)&userTime) - (*(ULONGLONG*)&prevUserTime);
    ULONGLONG totalSystem = kernelDiff + userDiff;

    CPUUtilization = (totalSystem - idleDiff) * 100.0 / totalSystem;

    return CPUUtilization;
};

//RAM - Total
double stats::GETRAMTotal(){
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    RAMTotal =  static_cast<double>(totalPhysMem) / (1024.0 * 1024 * 1024);
    return RAMTotal;
};

//RAM - Used
double stats::GETRAMUsed(){
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG PhysMemused = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    RAMUsed =  static_cast<double>(PhysMemused) / (1024.0 * 1024 * 1024);
    return RAMUsed;
};

//RAM - Utilization
double stats::GETRAMUtilization(){
    RAMUtilization = (GETRAMUsed()/GETRAMTotal()) * 100;
    return RAMUtilization;
};

//DISK - Total
double stats::GETDISKTotal(){
    LPCWSTR drive = L"C:\\";
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    GetDiskFreeSpaceExW(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes);
    DISKTotal = static_cast<double>(totalBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0);
    return DISKTotal;
};

//DISK - Used
double stats::GETDISKUsed(){
    LPCWSTR drive = L"C:\\";
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    GetDiskFreeSpaceExW(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes);
    ULONGLONG usedBytes = totalBytes.QuadPart - totalFreeBytes.QuadPart;
    DISKUsed = static_cast<double>(usedBytes) / (1024.0 * 1024.0 * 1024.0);
    return DISKUsed;
};

//DISK - Utilization
double stats::GETDISKUtilization(){
    DISKUtilization = (GETDISKUsed()/GETDISKTotal()) * 100;
    return DISKUtilization;
};
// NETWORK - Wi-Fi Send
double stats::GETWiFiSend() {
    PDH_HQUERY hQuery = nullptr;
    PDH_HCOUNTER hCounterWiFi;
    PDH_FMT_COUNTERVALUE counterValWiFi;
    DWORD dwCounterType;

    NETWORKSourceSend = "Wi-Fi";
    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) {
        std::cerr << "Failed to open query." << std::endl;
        PdhCloseQuery(hQuery);
        return 0.0;
    }

    if (PdhAddCounterW(hQuery, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes Sent/sec", 0, &hCounterWiFi) != ERROR_SUCCESS) {
        std::cerr << "Failed to add Wi-Fi counter." << std::endl;
        PdhCloseQuery(hQuery);
        return 0.0;
    }

    for (int i = 0; i < 2; i++) {
        Sleep(1000);
        if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) {
            std::cerr << "Failed to collect data." << std::endl;
            PdhCloseQuery(hQuery);
            return 0.0;
        }
    }

    if (PdhGetFormattedCounterValue(hCounterWiFi, PDH_FMT_DOUBLE, &dwCounterType, &counterValWiFi) == ERROR_SUCCESS) {
        double wifiKbps = (counterValWiFi.doubleValue * 8) / 1000;
        WiFiSend = wifiKbps;
    } else {
        std::cerr << "Failed to retrieve Wi-Fi counter value." << std::endl;
    }

    PdhCloseQuery(hQuery);
    return WiFiSend;
}

// NETWORK - Ethernet Send
double stats::GETEthernetSend() {
    PDH_HQUERY hQuery = nullptr;
    PDH_HCOUNTER hCounterEthernet;
    PDH_FMT_COUNTERVALUE counterValEthernet;
    DWORD dwCounterType;

    NETWORKSourceSend = "Ethernet";
    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) {
        std::cerr << "Failed to open query." << std::endl;
        PdhCloseQuery(hQuery);
        return 0.0;
    }

    if (PdhAddCounterW(hQuery, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Sent/sec", 0, &hCounterEthernet) != ERROR_SUCCESS) {
        std::cerr << "Failed to add Ethernet counter." << std::endl;
        PdhCloseQuery(hQuery);
        return 0.0;
    }

    for (int i = 0; i < 2; i++) {
        Sleep(1000);
        if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) {
            std::cerr << "Failed to collect data." << std::endl;
            PdhCloseQuery(hQuery);
            return 0.0;
        }
    }

    if (PdhGetFormattedCounterValue(hCounterEthernet, PDH_FMT_DOUBLE, &dwCounterType, &counterValEthernet) == ERROR_SUCCESS) {
        double ethernetKbps = (counterValEthernet.doubleValue * 8) / 1000;
        EthernetSend = ethernetKbps;
    } else {
        std::cerr << "Failed to retrieve Ethernet counter value." << std::endl;
    }

    PdhCloseQuery(hQuery);
    return EthernetSend;
}

// NETWORK - Wi-Fi Receive
double stats::GETWiFiReceive() {
    PDH_HQUERY hQuery = nullptr;
    PDH_HCOUNTER hCounterWiFi;
    PDH_FMT_COUNTERVALUE counterValWiFi;
    DWORD dwCounterType;

    NETWORKSourceReceive = "Wi-Fi";
    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) {
        std::cerr << "Failed to open query." << std::endl;
        PdhCloseQuery(hQuery);
        return 0.0;
    }

    if (PdhAddCounterW(hQuery, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes Received/sec", 0, &hCounterWiFi) != ERROR_SUCCESS) {
        std::cerr << "Failed to add Wi-Fi counter." << std::endl;
        PdhCloseQuery(hQuery);
        return 0.0;
    }

    for (int i = 0; i < 2; i++) {
        Sleep(1000);
        if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) {
            std::cerr << "Failed to collect data." << std::endl;
            PdhCloseQuery(hQuery);
            return 0.0;
        }
    }

    if (PdhGetFormattedCounterValue(hCounterWiFi, PDH_FMT_DOUBLE, &dwCounterType, &counterValWiFi) == ERROR_SUCCESS) {
        double wifiKbps = (counterValWiFi.doubleValue * 8) / 1000;
        WiFiReceive = wifiKbps;
    } else {
        std::cerr << "Failed to retrieve Wi-Fi counter value." << std::endl;
    }

    PdhCloseQuery(hQuery);
    return WiFiReceive;
}

// NETWORK - Ethernet Receive
double stats::GETEthernetReceive() {
    PDH_HQUERY hQuery = nullptr;
    PDH_HCOUNTER hCounterEthernet;
    PDH_FMT_COUNTERVALUE counterValEthernet;
    DWORD dwCounterType;

    NETWORKSourceReceive = "Ethernet";
    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) {
        std::cerr << "Failed to open query." << std::endl;
        PdhCloseQuery(hQuery);
        return 0.0;
    }

    if (PdhAddCounterW(hQuery, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Received/sec", 0, &hCounterEthernet) != ERROR_SUCCESS) {
        std::cerr << "Failed to add Ethernet counter." << std::endl;
        PdhCloseQuery(hQuery);
        return 0.0;
    }

    for (int i = 0; i < 2; i++) {
        Sleep(1000);
        if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) {
            std::cerr << "Failed to collect data." << std::endl;
            PdhCloseQuery(hQuery);
            return 0.0;
        }
    }

    if (PdhGetFormattedCounterValue(hCounterEthernet, PDH_FMT_DOUBLE, &dwCounterType, &counterValEthernet) == ERROR_SUCCESS) {
        double ethernetKbps = (counterValEthernet.doubleValue * 8) / 1000;
        EthernetReceive = ethernetKbps;
    } else {
        std::cerr << "Failed to retrieve Ethernet counter value." << std::endl;
    }

    PdhCloseQuery(hQuery);
    return EthernetReceive;
}
