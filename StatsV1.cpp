#include "StatsV1.h"

#include <iostream>
#include <string>
#include <windows.h>
#include <intrin.h>
#include <pdh.h>
#include <pdhmsg.h>

#ifdef _MSC_VER
#pragma comment(lib, "pdh.lib")
#endif

// Constructor
StatsV1::StatsV1() {
    // Initialize CPU frequency
    this->CPUFrequency = 0.0;
    this->CPUUtilization = 0.0;
    QueryPerformanceFrequency(&frequency);

    // Initialize RAM
    this->RAMTotal = 0.0;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        this->RAMTotal = static_cast<float>(memInfo.ullTotalPhys) / (1024.0 * 1024 * 1024);
    } else {
        std::cerr << "Failed to retrieve memory status." << std::endl;
    }

    // Initialize Disk
    this->DISKTotal = 0.0;
    if (GetDiskFreeSpaceExW(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        this->DISKTotal = static_cast<double>(totalBytes.QuadPart) / (1024.0 * 1024.0 * 1024.0);
    } else {
        std::cerr << "Failed to retrieve disk space." << std::endl;
    }

    // Initialize Network - Wi-Fi
    if (PdhOpenQuery(NULL, 0, &hQueryWiFi) == ERROR_SUCCESS) {
        if (PdhAddCounterW(hQueryWiFi, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes Sent/sec", 0, &hCounterWiFiSend) != ERROR_SUCCESS) {
            std::cerr << "Failed to add Wi-Fi send counter." << std::endl;
        }
        if (PdhAddCounterW(hQueryWiFi, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes Received/sec", 0, &hCounterWiFiReceive) != ERROR_SUCCESS) {
            std::cerr << "Failed to add Wi-Fi receive counter." << std::endl;
        }
    } else {
        std::cerr << "Failed to open Wi-Fi query." << std::endl;
    }
    this->WiFiSend = 0.0;
    this->WiFiReceive = 0.0;

    // Initialize Network - Ethernet
    if (PdhOpenQuery(NULL, 0, &hQueryEthernet) == ERROR_SUCCESS) {
        if (PdhAddCounterW(hQueryEthernet, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Sent/sec", 0, &hCounterEthernetSend) != ERROR_SUCCESS) {
            std::cerr << "Failed to add Ethernet send counter." << std::endl;
        }
        if (PdhAddCounterW(hQueryEthernet, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Received/sec", 0, &hCounterEthernetReceive) != ERROR_SUCCESS) {
            std::cerr << "Failed to add Ethernet receive counter." << std::endl;
        }
    } else {
        std::cerr << "Failed to open Ethernet query." << std::endl;
    }
    this->EthernetSend = 0.0;
    this->EthernetReceive = 0.0;
}

// Destructor
StatsV1::~StatsV1() {
    // Check if queries and counters are valid before removing them
    if (hQueryWiFi) {
        if (hCounterWiFiSend) PdhRemoveCounter(hCounterWiFiSend);
        if (hCounterWiFiReceive) PdhRemoveCounter(hCounterWiFiReceive);
        PdhCloseQuery(hQueryWiFi);
    }

    if (hQueryEthernet) {
        if (hCounterEthernetSend) PdhRemoveCounter(hCounterEthernetSend);
        if (hCounterEthernetReceive) PdhRemoveCounter(hCounterEthernetReceive);
        PdhCloseQuery(hQueryEthernet);
    }
}

// CPU - Frequency
float StatsV1::GetCPUFrequency() {
    QueryPerformanceCounter(&start);
    startTime = __rdtsc();
    Sleep(500);
    endTime = __rdtsc();
    QueryPerformanceCounter(&end);

    timeDiff = static_cast<float>(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    CPUFrequency = static_cast<float>(endTime - startTime) / (timeDiff * 1000000 * 2);
    if (CPUFrequency < 0) CPUFrequency = 0.0;

    return CPUFrequency;
}

// CPU - Utilization
float StatsV1::GetCPUUtilization() {
    GetSystemTimes(&prevIdleTime, &prevKernelTime, &prevUserTime);
    Sleep(500);
    GetSystemTimes(&idleTime, &kernelTime, &userTime);

    idleDiff = (*(ULONGLONG*)&idleTime) - (*(ULONGLONG*)&prevIdleTime);
    kernelDiff = (*(ULONGLONG*)&kernelTime) - (*(ULONGLONG*)&prevKernelTime);
    userDiff = (*(ULONGLONG*)&userTime) - (*(ULONGLONG*)&prevUserTime);
    totalSystem = kernelDiff + userDiff;

    CPUUtilization = (totalSystem - idleDiff) * 100.0 / totalSystem;
    return CPUUtilization;
}

// RAM - Total
float StatsV1::GETRAMTotal() { return RAMTotal; }

// RAM - Used
float StatsV1::GETRAMUsed() {
    // Use the class member `memInfo` instead of declaring a new one
    DWORDLONG PhysMemused = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    RAMUsed = static_cast<float>(PhysMemused) / (1024.0 * 1024 * 1024);
    return RAMUsed;
}

// RAM - Utilization
float StatsV1::GETRAMUtilization() {
    RAMUtilization = (GETRAMUsed() / GETRAMTotal()) * 100;
    return RAMUtilization;
}

// DISK - Total
float StatsV1::GETDISKTotal() { return DISKTotal; }

// DISK - Used
float StatsV1::GETDISKUsed() {
    // Query disk space once per function call instead of multiple times
    GetDiskFreeSpaceExW(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes);
    usedBytes = totalBytes.QuadPart - totalFreeBytes.QuadPart;
    DISKUsed = static_cast<float>(usedBytes) / (1024.0 * 1024.0 * 1024.0);
    return DISKUsed;
}

// DISK - Utilization
float StatsV1::GETDISKUtilization() {
    DISKUtilization = (GETDISKUsed() / GETDISKTotal()) * 100;
    return DISKUtilization;
}

// NETWORK - Wi-Fi Send
float StatsV1::GETWiFiSend() {
    if (PdhCollectQueryData(hQueryWiFi) != ERROR_SUCCESS) {
        std::cerr << "Failed to collect Wi-Fi data." << std::endl;
        return 0.0;
    }

    if (PdhGetFormattedCounterValue(hCounterWiFiSend, PDH_FMT_DOUBLE, &dwCounterTypeWiFiSend, &counterValWiFiSend) == ERROR_SUCCESS) {
        float wifiKbps = (counterValWiFiSend.doubleValue * 8) / 1000;
        WiFiSend = wifiKbps;
    } else {
        std::cerr << "Failed to retrieve Wi-Fi send counter value." << std::endl;
    }

    return WiFiSend;
}

// NETWORK - Wi-Fi Receive
float StatsV1::GETWiFiReceive() {
    if (PdhCollectQueryData(hQueryWiFi) != ERROR_SUCCESS) {
        std::cerr << "Failed to collect Wi-Fi data." << std::endl;
        return 0.0;
    }

    if (PdhGetFormattedCounterValue(hCounterWiFiReceive, PDH_FMT_DOUBLE, &dwCounterTypeWiFiReceive, &counterValWiFiReceive) == ERROR_SUCCESS) {
        float wifiKbps = (counterValWiFiReceive.doubleValue * 8) / 1000;
        WiFiReceive = wifiKbps;
    } else {
        std::cerr << "Failed to retrieve Wi-Fi receive counter value." << std::endl;
    }

    return WiFiReceive;
}

// NETWORK - Ethernet Send
float StatsV1::GETEthernetSend() {
    if (PdhCollectQueryData(hQueryEthernet) != ERROR_SUCCESS) {
        std::cerr << "Failed to collect Ethernet data." << std::endl;
        return 0.0;
    }

    if (PdhGetFormattedCounterValue(hCounterEthernetSend, PDH_FMT_DOUBLE, &dwCounterTypeEthernetSend, &counterValEthernetSend) == ERROR_SUCCESS) {
        float ethernetKbps = (counterValEthernetSend.doubleValue * 8) / 1000;
        EthernetSend = ethernetKbps;
    } else {
        std::cerr << "Failed to retrieve Ethernet send counter value." << std::endl;
    }

    return EthernetSend;
}

// NETWORK - Ethernet Receive
float StatsV1::GETEthernetReceive() {
    if (PdhCollectQueryData(hQueryEthernet) != ERROR_SUCCESS) {
        std::cerr << "Failed to collect Ethernet data." << std::endl;
        return 0.0;
    }

    if (PdhGetFormattedCounterValue(hCounterEthernetReceive, PDH_FMT_DOUBLE, &dwCounterTypeEthernetReceive, &counterValEthernetReceive) == ERROR_SUCCESS) {
        float ethernetKbps = (counterValEthernetReceive.doubleValue * 8) / 1000;
        EthernetReceive = ethernetKbps;
    } else {
        std::cerr << "Failed to retrieve Ethernet receive counter value." << std::endl;
    }

    return EthernetReceive;
}
