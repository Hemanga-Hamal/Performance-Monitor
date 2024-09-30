#include <iostream>
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <conio.h>
#include <iomanip> 

#pragma comment(lib, "pdh.lib")

int main() {
    PDH_HQUERY hQuery;
    PDH_HCOUNTER hCounterWiFi, hCounterEthernet;
    PDH_FMT_COUNTERVALUE counterValWiFi, counterValEthernet;
    DWORD dwCounterType;

    // Initialize the query
    if (PdhOpenQuery(NULL, 0, &hQuery) != ERROR_SUCCESS) {
        std::cerr << "Failed to open query." << std::endl;
        return 1;
    }

    // Add counter for Wi-Fi Adapter (MediaTek Wi-Fi 6 MT7921 Wireless LAN Card) - Bytes Received/sec
    if (PdhAddCounterW(hQuery, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes Received/sec", 0, &hCounterWiFi) != ERROR_SUCCESS) {
        std::cerr << "Failed to add Wi-Fi counter." << std::endl;
    }

    // Add counter for Ethernet Adapter (Realtek PCIe GBE Family Controller) - Bytes Received/sec
    if (PdhAddCounterW(hQuery, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes Received/sec", 0, &hCounterEthernet) != ERROR_SUCCESS) {
        std::cerr << "Failed to add Ethernet counter." << std::endl;
    }

    std::cout << "Press Enter to stop..." << std::endl;

    while (!_kbhit() || _getch() != '\r') {
        // Collect data
        if (PdhCollectQueryData(hQuery) != ERROR_SUCCESS) {
            std::cerr << "Failed to collect data." << std::endl;
            break;
        }

        // Retrieve and format the counter value for Wi-Fi (received)
        if (PdhGetFormattedCounterValue(hCounterWiFi, PDH_FMT_DOUBLE, &dwCounterType, &counterValWiFi) == ERROR_SUCCESS) {
            double wifiKbps = (counterValWiFi.doubleValue * 8) / 1000;  // Convert to Kbps
            double wifiMbps = wifiKbps / 1000;                          // Convert to Mbps

            if (wifiKbps > 0) {
                std::cout << "Active Adapter: Wi-Fi" << std::endl;
                if (wifiMbps >= 1) {
                    std::cout << "Wi-Fi receive: " << std::fixed << std::setprecision(4) << wifiMbps << " Mbps" << std::endl;
                } else {
                    std::cout << "Wi-Fi receive: " << std::fixed << std::setprecision(4) << wifiKbps << " Kbps" << std::endl;
                }
            }
        } else {
            std::cerr << "Failed to retrieve Wi-Fi counter value." << std::endl;
        }

        // Retrieve and format the counter value for Ethernet (received)
        if (PdhGetFormattedCounterValue(hCounterEthernet, PDH_FMT_DOUBLE, &dwCounterType, &counterValEthernet) == ERROR_SUCCESS) {
            double ethernetKbps = (counterValEthernet.doubleValue * 8) / 1000;  // Convert to Kbps
            double ethernetMbps = ethernetKbps / 1000;                          // Convert to Mbps

            if (ethernetKbps > 0) {
                std::cout << "Active Adapter: Ethernet" << std::endl;
                if (ethernetMbps >= 1) {
                    std::cout << "Ethernet receive: " << std::fixed << std::setprecision(4) << ethernetMbps << " Mbps" << std::endl;
                } else {
                    std::cout << "Ethernet receive: " << std::fixed << std::setprecision(4) << ethernetKbps << " Kbps" << std::endl;
                }
            }
        } else {
            std::cerr << "Failed to retrieve Ethernet counter value." << std::endl;
        }

        Sleep(1000);
    }

    PdhCloseQuery(hQuery);

    return 0;
}
