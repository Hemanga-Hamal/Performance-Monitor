#include <windows.h>    // For Windows-specific APIs
#include <pdh.h>        // For Performance Data Helper (PDH) functions -> need to add  -lpdh to compile with lib
#include <pdhmsg.h>     // For PDH error messages and status codes
#include <iostream>     // For input/output stream operations
#include <conio.h>      // For _kbhit() and _getch(), to detect key presses

int main() {
    PDH_HQUERY query;               // Handle to the query
    PDH_HCOUNTER counter;           // Handle to the counter (CPU usage)
    PDH_FMT_COUNTERVALUE counterVal;// Struct to store the formatted counter value
    
    // Open a query to monitor the CPU usage
    // The first two parameters can be NULL, as we don't need custom data sources or user data
    PdhOpenQuery(NULL, NULL, &query);
    
    // Add a counter to the query to track the total processor time (% CPU usage)
    // The counter path is "\\Processor(_Total)\\% Processor Time"
    // It monitors the total CPU usage across all cores
    PdhAddCounterW(query, L"\\Processor(_Total)\\% Processor Time", NULL, &counter);
    
    std::cout << "Press Enter to stop monitoring..." << std::endl;
    
    // Loop until the Enter key is pressed
    while (true) {
        // Collects the initial data for the query (needed before getting any values)
        PdhCollectQueryData(query);
        
        Sleep(500);  // Wait for a second to collect more accurate readings
        
        // Collect the query data again after the sleep period
        PdhCollectQueryData(query);
        
        // Get the formatted value of the counter as a double (floating-point number)
        PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &counterVal);
        
        // Output the current CPU utilization
        std::cout << "CPU Utilization: " << counterVal.doubleValue << "%" << std::endl;
        
        // Check if any key has been pressed
        if (_kbhit()) {
            // If the Enter key ('\r') is pressed, break out of the loop
            if (_getch() == '\r') {
                break;
            }
        }
    }
    
    // Close the query to free up resources
    PdhCloseQuery(query);
    return 0;
}
