#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iostream>
#include <conio.h> // For _kbhit() and _getch()

int main() {
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE counterVal;
    
    // Open a query to monitor the CPU usage
    PdhOpenQuery(NULL, NULL, &query);
    PdhAddCounterW(query, L"\\Processor(_Total)\\% Processor Time", NULL, &counter);
    
    std::cout << "Press Enter to stop monitoring..." << std::endl;
    
    // Loop until the Enter key is pressed
    while (true) {
        PdhCollectQueryData(query);
        
        Sleep(1000);  // Wait for a second for more accurate reading
        
        PdhCollectQueryData(query);
        PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterVal);
        
        std::cout << "CPU Utilization: " << counterVal.doubleValue << "%" << std::endl;
        
        // Check if the Enter key was pressed
        if (_kbhit()) {
            if (_getch() == '\r') { // '\r' corresponds to Enter key
                break;
            }
        }
    }
    
    PdhCloseQuery(query);
    return 0;
}
