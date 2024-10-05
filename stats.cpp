#include "stats.h"


#include <iostream>
#include <string>
#include <windows.h>
#include <intrin.h>
#include <conio.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iomanip> 

#pragma comment(lib, "pdh.lib")

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
    //NETWORK
    this->NETWORKSend = 0.0;
    this->NETWORKRecieve = 0.0;
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

double stats::GETNETWORKSend() {
    PDH_HQUERY hQuery;
    PDH_HCOUNTER hCounterWiFi, hCounterEthernet;
    PDH_FMT_COUNTERVALUE counterValWiFi, counterValEthernet;
    DWORD dwCounterType;

    double sendKbps = 0.0;

    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) {
        return sendKbps;
    }

    if (PdhAddCounterW(hQuery, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes Sent/sec", 0, &hCounterWiFi) != ERROR_SUCCESS) {
        PdhCloseQuery(hQuery);
        return sendKbps;
    }

    if (PdhAddCounterW(hQuery, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Sent/sec", 0, &hCounterEthernet) != ERROR_SUCCESS) {
        PdhCloseQuery(hQuery);
        return sendKbps;
    }

    // Allow some time for data to accumulate
    Sleep(1000);

    // Collect data
    if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) {
        PdhCloseQuery(hQuery);
        return sendKbps;
    }

    // Retrieve the counter value for Wi-Fi
    if (PdhGetFormattedCounterValue(hCounterWiFi, PDH_FMT_DOUBLE, &dwCounterType, &counterValWiFi) == ERROR_SUCCESS) {
        double wifiKbps = (counterValWiFi.doubleValue * 8) / 1000;
        if (wifiKbps > 0) {
            sendKbps = wifiKbps;
            this->NETWORKSourceSend = "Wi-Fi";
        }
    }

    // Retrieve the counter value for Ethernet
    if (PdhGetFormattedCounterValue(hCounterEthernet, PDH_FMT_DOUBLE, &dwCounterType, &counterValEthernet) == ERROR_SUCCESS) {
        double ethernetKbps = (counterValEthernet.doubleValue * 8) / 1000;
        if (ethernetKbps > 0) {
            sendKbps = ethernetKbps;
            this->NETWORKSourceSend = "Ethernet";
        }
    }

    PdhCloseQuery(hQuery);
    return sendKbps; // Returns the last valid send rate
}

//NETWORK - Receive
double stats::GETNETWORKReceive() {
    PDH_HQUERY hQuery;
    PDH_HCOUNTER hCounterWiFi, hCounterEthernet;
    PDH_FMT_COUNTERVALUE counterValWiFi, counterValEthernet;
    DWORD dwCounterType;

    double receiveKbps = 0.0;
    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) {return receiveKbps;}
    if (PdhAddCounterW(hQuery, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes Received/sec", 0, &hCounterWiFi) != ERROR_SUCCESS) 
    {return receiveKbps;}
    if (PdhAddCounterW(hQuery, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Received/sec", 0, &hCounterEthernet) != ERROR_SUCCESS) 
    {return receiveKbps;}

    if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) {
        PdhCloseQuery(hQuery);
        return receiveKbps;
    }

    if (PdhGetFormattedCounterValue(hCounterWiFi, PDH_FMT_DOUBLE, &dwCounterType, &counterValWiFi) == ERROR_SUCCESS) {
        double wifiKbps = (counterValWiFi.doubleValue * 8) / 1000;
        if (wifiKbps > 0) {
            this->NETWORKRecieve = wifiKbps;
            this->NETWORKSourceRecieved = "Wi-Fi";
        }
    }

    if (PdhGetFormattedCounterValue(hCounterEthernet, PDH_FMT_DOUBLE, &dwCounterType, &counterValEthernet) == ERROR_SUCCESS) {
        double ethernetKbps = (counterValEthernet.doubleValue * 8) / 1000;
        if (ethernetKbps > 0) {
            this->NETWORKRecieve = ethernetKbps;
            this->NETWORKSourceRecieved = "Ethernet";
        }
    }

    PdhCloseQuery(hQuery);
    return NETWORKRecieve;
}