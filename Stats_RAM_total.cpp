#include <windows.h>
#include <iostream>

int main() {
    // Create a MEMORYSTATUSEX structure
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    // Call GlobalMemoryStatusEx to fill the structure with memory information
    if (GlobalMemoryStatusEx(&memInfo)) {
        // Total physical memory in bytes
        DWORDLONG totalPhysMem = memInfo.ullTotalPhys;

        // Convert bytes to gigabytes for easier reading
        double totalPhysMemGB = totalPhysMem / (1024.0 * 1024 * 1024);

        // Output the total physical memory
        std::cout << "Total Physical Memory: " << totalPhysMemGB << " GB" << std::endl;
    } else {
        // Handle error
        std::cerr << "Failed to retrieve memory information." << std::endl;
        return 1;
    }

    return 0;
}
