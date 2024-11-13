#include <iostream>
#include <windows.h>
#include <Pdh.h>  // Link to Pdh.lib

#pragma comment(lib, "Pdh.lib")

#define YOUR_CPU_MAX_FREQUENCY 3.3  // Max CPU frequency in GHz

int main() {
    // Declare query and counter handles
    HQUERY hquery;
    HCOUNTER hcounter;
    
    // Open the query
    PDH_STATUS status = PdhOpenQueryA(nullptr, NULL, &hquery);
    if (status != ERROR_SUCCESS) {
        std::cerr << "Failed to open query: " << status << std::endl;
        return 1;
    }

    // Add the counter for the CPU usage (percentage of processor performance)
    status = PdhAddCounterA(hquery, "\\Processor Information(_Total)\\% Processor Performance", NULL, &hcounter);
    if (status != ERROR_SUCCESS) {
        std::cerr << "Failed to add counter: " << status << std::endl;
        PdhCloseQuery(hquery);
        return 1;
    }

    // Continuous loop
    while (true) {
        // Collect data
        status = PdhCollectQueryData(hquery);
        if (status != ERROR_SUCCESS) {
            std::cerr << "Failed to collect query data: " << status << std::endl;
            PdhCloseQuery(hquery);
            return 1;
        }

        // Sleep for 200 ms before collecting data again
        Sleep(200);

        // Collect the data again
        status = PdhCollectQueryData(hquery);
        if (status != ERROR_SUCCESS) {
            std::cerr << "Failed to collect query data: " << status << std::endl;
            PdhCloseQuery(hquery);
            return 1;
        }

        // Retrieve the formatted counter value (CPU usage percentage)
        PDH_FMT_COUNTERVALUE value;
        status = PdhGetFormattedCounterValue(hcounter, PDH_FMT_DOUBLE, nullptr, &value);
        if (status != ERROR_SUCCESS) {
            std::cerr << "Failed to get formatted counter value: " << status << std::endl;
            PdhCloseQuery(hquery);
            return 1;
        }

        // Calculate the CPU frequency based on the percentage
        double cpuUsage = value.doubleValue;  // CPU usage percentage
        double cpuFrequency = cpuUsage / 100 * YOUR_CPU_MAX_FREQUENCY;

        // Output the result
        std::cout << "CPU Usage: " << cpuUsage << "%" << std::endl;
        std::cout << "Calculated CPU Frequency: " << cpuFrequency << " GHz" << std::endl;

        // Optional: Add a short sleep before next iteration (to avoid overwhelming the system)
        Sleep(800);  // Sleep for 1 second before the next iteration
    }

    // Close the query (this won't be reached because of the infinite loop)
    PdhCloseQuery(hquery);

    return 0;
}
