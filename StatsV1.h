#ifndef STATSV1_H
#define STATSV1_H

#include <iostream>
#include <string>
#include <windows.h>
#include <intrin.h>
#include <conio.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iomanip> 
#include <limits>

class StatsV1 {
private:
    // CPU - Frequency
    LARGE_INTEGER frequency, start, end;
    DWORD64 startTime, endTime;
    float timeDiff;
    float CPUFrequency;
    // CPU - Utilization
    FILETIME idleTime, kernelTime, userTime;
    FILETIME prevIdleTime, prevKernelTime, prevUserTime;
    ULONGLONG idleDiff, kernelDiff, userDiff, totalSystem;
    float CPUUtilization;
    // RAM - Total
    MEMORYSTATUSEX memInfo;
    float RAMTotal;
    // RAM - Used
    float RAMUsed;
    // RAM - Utilization
    float RAMUtilization;
    // DISK - Total
    LPCWSTR drive = L"C:\\";
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    float DISKTotal;
    //Disk - Used
    ULONGLONG usedBytes;
    float DISKUsed;
    // DISK - Utilization
    float DISKUtilization;
    // NETWORK - Wi-Fi and Ethernet Send/Receive Queries
    PDH_HQUERY hQueryWiFi, hQueryEthernet;
    PDH_HCOUNTER hCounterWiFiSend, hCounterWiFiReceive;
    PDH_HCOUNTER hCounterEthernetSend, hCounterEthernetReceive;
    PDH_FMT_COUNTERVALUE counterValWiFiSend;
    PDH_FMT_COUNTERVALUE counterValWiFiReceive;
    PDH_FMT_COUNTERVALUE counterValEthernetSend;
    PDH_FMT_COUNTERVALUE counterValEthernetReceive;
    DWORD dwCounterTypeWiFiSend;
    DWORD dwCounterTypeWiFiReceive;
    DWORD dwCounterTypeEthernetSend;
    DWORD dwCounterTypeEthernetReceive;
    float WiFiSend, WiFiReceive;
    float EthernetSend, EthernetReceive;


public:
    // Constructor and Destructor
    StatsV1();
    ~StatsV1();

    // CPU
    float GetCPUFrequency();
    float GetCPUUtilization();

    // RAM
    float GETRAMTotal();
    float GETRAMUsed();
    float GETRAMUtilization();

    // DISK
    float GETDISKTotal();
    float GETDISKUsed();
    float GETDISKUtilization();

    // NETWORK
    float GETWiFiSend();
    float GETWiFiReceive();
    float GETEthernetSend();
    float GETEthernetReceive();

};

#endif // STATS_H