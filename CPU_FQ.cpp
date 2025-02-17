#include <windows.h>
#include <iostream>

int main() {
    LARGE_INTEGER frequency, start, end;
    DWORD64 startTime, endTime;

    // Get the frequency of the high-resolution performance counter
    QueryPerformanceFrequency(&frequency);

    // Get the start count
    QueryPerformanceCounter(&start);
    startTime = __rdtsc(); // Read the time-stamp counter

    // Introduce a small delay to measure the frequency
    Sleep(1000); // Sleep for 100 milliseconds

    // Get the end count
    endTime = __rdtsc(); // Read the time-stamp counter again
    QueryPerformanceCounter(&end);

    // Calculate the time difference in seconds
    double timeDiff = static_cast<double>(end.QuadPart - start.QuadPart) / frequency.QuadPart;

    // Calculate the CPU frequency
    double cpuFrequency = static_cast<double>(endTime - startTime) / (timeDiff * 1000000); // In MHz

    std::cout << "Real-time CPU Frequency: " << cpuFrequency << " MHz" << std::endl;

    return 0;
}