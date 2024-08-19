#include <windows.h>
#include <iostream>

int main() {
    // Create a MEMORYSTATUSEX structure to hold memory status information
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);  // Set the structure size

    // Retrieve the system's current memory status
    if (GlobalMemoryStatusEx(&memInfo)) {

        // Calculate the amount of used physical memory
        DWORDLONG PhysMemused = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
        double PhysMemusedGB = PhysMemused / (1024.0 * 1024 * 1024);  // Convert bytes to gigabytes

        // Output the used physical memory in GB
        std::cout << "Total Physical Memory: " << PhysMemusedGB << " GB" << std::endl;

    } else {
        // Handle the case where memory information could not be retrieved
        std::cerr << "Failed to retrieve memory information and calculate memory utilization." << std::endl;
        return 1; 
    }

    return 0; 
}
