#include <windows.h>
#include <iostream>
#include <conio.h>

int main() {
    FILETIME idleTime, kernelTime, userTime;
    FILETIME prevIdleTime, prevKernelTime, prevUserTime;

    // Get initial times
    GetSystemTimes(&prevIdleTime, &prevKernelTime, &prevUserTime);

    std::cout << "Press Enter to stop monitoring..." << std::endl;

    while (true) {
        Sleep(1000); // Wait for 1 second

        // Get new times
        GetSystemTimes(&idleTime, &kernelTime, &userTime);

        // Calculate differences
        ULONGLONG idleDiff = (*(ULONGLONG*)&idleTime) - (*(ULONGLONG*)&prevIdleTime);
        ULONGLONG kernelDiff = (*(ULONGLONG*)&kernelTime) - (*(ULONGLONG*)&prevKernelTime);
        ULONGLONG userDiff = (*(ULONGLONG*)&userTime) - (*(ULONGLONG*)&prevUserTime);

        // Calculate CPU usage
        ULONGLONG totalSystem = kernelDiff + userDiff;
        double cpuUsage = (totalSystem - idleDiff) * 100.0 / totalSystem;

        // Output CPU usage
        std::cout << "CPU Utilization: " << cpuUsage << "%" << std::endl;

        // Update previous times
        prevIdleTime = idleTime;
        prevKernelTime = kernelTime;
        prevUserTime = userTime;

        // Check for Enter key press
        if (_kbhit() && _getch() == '\r') {
            break;
        }
    }

    return 0;
}
