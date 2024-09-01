//SEE WINDOWS PERFORMANCE COUNTER

#include <windows.h>
#include <pdh.h>
#include <iostream>
#include <string>

#pragma comment(lib, "pdh.lib")

int main() {
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE counterVal;

    std::string networkInterface = "Ethernet";  // Replace with your network interface name
    std::wstring counterPath = L"\\Network Interface(" + std::wstring(networkInterface.begin(), networkInterface.end()) + L")\\Bytes Total/sec";

    // Open a query object.
    PdhOpenQuery(NULL, NULL, &query);

    // Add the counter to the query.
    PdhAddCounterW(query, counterPath.c_str(), NULL, &counter);

    // Collect data for the first time.
    PdhCollectQueryData(query);

    // Sleep for 1 second to get the network speed.
    Sleep(1000);

    // Collect data again.
    PdhCollectQueryData(query);

    // Get the formatted counter value.
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterVal);

    // Close the query.
    PdhCloseQuery(query);

    // Display network speed in KBps
    std::cout << "Network speed: " << counterVal.doubleValue / 1024.0 << " KBps" << std::endl;

    return 0;
}
