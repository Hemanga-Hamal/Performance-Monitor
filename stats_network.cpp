#include "stats.h"

#include <iostream>
#include <string>
#include <windows.h>
#include <intrin.h>
#include <conio.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iomanip> 
#include <limits>

#pragma comment(lib, "pdh.lib")

// NETWORK - Wi-Fi Send
double stats::GETWiFiSend(){
    PDH_HQUERY hQuery;
    PDH_HCOUNTER hCounterWiFi;
    PDH_FMT_COUNTERVALUE counterValWiFi;
    DWORD dwCounterType;

    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) return 0.0;

    if (PdhAddCounterW(hQuery, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes Sent/sec", 0, &hCounterWiFi) != ERROR_SUCCESS) return 0.0;

    if (PdhCollectQueryData(hQuery) == ERROR_SUCCESS) {
        if (PdhGetFormattedCounterValue(hCounterWiFi, PDH_FMT_DOUBLE, &dwCounterType, &counterValWiFi) == ERROR_SUCCESS) {
            WiFiSend = (counterValWiFi.doubleValue * 8) / 1000;  // Convert to Kbps
            NETWORKSourceSend = "Wi-Fi";
        }
    }

    PdhCloseQuery(hQuery);
    return WiFiSend;
}

// NETWORK - Wi-Fi Receive
double stats::GETWiFiReceive(){
    PDH_HQUERY hQuery;
    PDH_HCOUNTER hCounterWiFi;
    PDH_FMT_COUNTERVALUE counterValWiFi;
    DWORD dwCounterType;

    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) return 0.0;

    if (PdhAddCounterW(hQuery, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes Received/sec", 0, &hCounterWiFi) != ERROR_SUCCESS) return 0.0;

    if (PdhCollectQueryData(hQuery) == ERROR_SUCCESS) {
        if (PdhGetFormattedCounterValue(hCounterWiFi, PDH_FMT_DOUBLE, &dwCounterType, &counterValWiFi) == ERROR_SUCCESS) {
            WiFiReceive = (counterValWiFi.doubleValue * 8) / 1000;  // Convert to Kbps
            NETWORKSourceReceive = "Wi-Fi";
        }
    }

    PdhCloseQuery(hQuery);
    return WiFiReceive;
}

// NETWORK - Ethernet Send
double stats::GETEthernetSend(){
    PDH_HQUERY hQuery;
    PDH_HCOUNTER hCounterEthernet;
    PDH_FMT_COUNTERVALUE counterValEthernet;
    DWORD dwCounterType;

    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) return 0.0;

    if (PdhAddCounterW(hQuery, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Sent/sec", 0, &hCounterEthernet) != ERROR_SUCCESS) return 0.0;

    if (PdhCollectQueryData(hQuery) == ERROR_SUCCESS) {
        if (PdhGetFormattedCounterValue(hCounterEthernet, PDH_FMT_DOUBLE, &dwCounterType, &counterValEthernet) == ERROR_SUCCESS) {
            EthernetSend = (counterValEthernet.doubleValue * 8) / 1000;  // Convert to Kbps
            NETWORKSourceSend = "Ethernet";
        }
    }

    PdhCloseQuery(hQuery);
    return EthernetSend;
}

// NETWORK - Ethernet Receive
double stats::GETEthernetReceive(){
    PDH_HQUERY hQuery;
    PDH_HCOUNTER hCounterEthernet;
    PDH_FMT_COUNTERVALUE counterValEthernet;
    DWORD dwCounterType;

    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) return 0.0;

    if (PdhAddCounterW(hQuery, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Received/sec", 0, &hCounterEthernet) != ERROR_SUCCESS) return 0.0;

    if (PdhCollectQueryData(hQuery) == ERROR_SUCCESS) {
        if (PdhGetFormattedCounterValue(hCounterEthernet, PDH_FMT_DOUBLE, &dwCounterType, &counterValEthernet) == ERROR_SUCCESS) {
            EthernetReceive = (counterValEthernet.doubleValue * 8) / 1000;  // Convert to Kbps
            NETWORKSourceReceive = "Ethernet";
        }
    }

    PdhCloseQuery(hQuery);
    return EthernetReceive;
}