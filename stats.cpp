#include "stats.h"

// Constructor
stats::stats(){
    //CPU
    this->CPUFrequency = 0.0;
    this->CPUUtilization = 0.0;
    //RAM
    this->RAMTotal = 0.0;
    this->RAMUsed = 0.0;
    this->RAMUtilization = 0.0;
};

// Destructor
stats::~stats(){};

//Cpu - Frequency
double stats::GetCPUFrequency(){
    LARGE_INTEGER frequency, start, end;
    DWORD64 startTime, endTime;

    QueryPerformanceFrequency(&frequency);
    startTime = __rdtsc();
    Sleep(1000);
    endTime = __rdtsc();
    QueryPerformanceCounter(&end);

    double timeDiff = static_cast<double>(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    CPUFrequency = static_cast<double>(endTime - startTime) / (timeDiff * 1000000);

    return CPUFrequency;
};

//Cpu - Utilization
double stats::GetCPUUtilization(){
    FILETIME idleTime, kernelTime, userTime;
    FILETIME prevIdleTime, prevKernelTime, prevUserTime;

    GetSystemTimes(&prevIdleTime, &prevKernelTime, &prevUserTime);
    Sleep(1000);
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
    
    ULONGLONG idleDiff = (*(ULONGLONG*)&idleTime) - (*(ULONGLONG*)&prevIdleTime);
    ULONGLONG kernelDiff = (*(ULONGLONG*)&kernelTime) - (*(ULONGLONG*)&prevKernelTime);
    ULONGLONG userDiff = (*(ULONGLONG*)&userTime) - (*(ULONGLONG*)&prevUserTime);
    ULONGLONG totalSystem = kernelDiff + userDiff;

    CPUUtilization = (totalSystem - idleDiff) * 100.0 / totalSystem;

    return CPUUtilization;
};

//RAM - Total
double stats::GETRAMTotal(){
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    RAMTotal =  static_cast<double>(totalPhysMem) / (1024.0 * 1024 * 1024);
    return RAMTotal;
};

//RAM - Used
double stats::GETRAMUsed(){
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG PhysMemused = memInfo.ullTotalPhys - memInfo.ullAvailPhys;
    RAMUsed =  static_cast<double>(PhysMemused) / (1024.0 * 1024 * 1024);
    return RAMUsed;
};

//RAM - Utilization
double stats::GETRAMUtilization(){
    RAMUtilization = (GETRAMTotal()/GETRAMTotal()) * 100;
    return RAMUtilization;
};