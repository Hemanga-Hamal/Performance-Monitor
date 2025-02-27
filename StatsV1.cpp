#include "StatsV1.h"
#include <intrin.h>
#include <system_error>
#include <pdh.h>

#ifdef _MSC_VER
#pragma comment(lib, "pdh.lib")
#endif

namespace {
    constexpr float BYTES_TO_GB = 1024.0f * 1024.0f * 1024.0f;
    constexpr float BYTES_TO_MBPS = 8.0f / 1e6;
    constexpr DWORD MEASUREMENT_DELAY = 200;
}

StatsV1::StatsV1() noexcept {
    // Initialize CPU frequency query
    if (PdhOpenQuery(nullptr, 0, &cpuQuery) == ERROR_SUCCESS) {
        // Use the correct counter path for CPU frequency
        if (PdhAddCounterW(cpuQuery, L"\\Processor Information(_Total)\\Processor Frequency", 0, &counterFreq) != ERROR_SUCCESS) {
            if (cpuQuery) {
                PdhCloseQuery(cpuQuery);
                cpuQuery = nullptr;
            }
        }

        if (PdhAddCounterW(cpuQuery, L"\\Processor Information(_Total)\\% Processor Performance", 0, &counterPerf) != ERROR_SUCCESS) {
            if (cpuQuery) {
                PdhCloseQuery(cpuQuery);
                cpuQuery = nullptr;
            }
        }
        // Initialize the query with first collection
        if (cpuQuery) {
            PdhCollectQueryData(cpuQuery);
        }
    }

    // Initialize RAM info
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        RAMTotal = static_cast<float>(memInfo.ullTotalPhys) / BYTES_TO_GB;
    }

    // Initialize Disk info
    const std::wstring DRIVE = L"C:\\";
    ULARGE_INTEGER totalBytes;
    if (GetDiskFreeSpaceExW(DRIVE.c_str(), nullptr, &totalBytes, nullptr)) {
        DISKTotal = static_cast<float>(totalBytes.QuadPart) / BYTES_TO_GB;
    }

    // Initialize Network counters
    InitializeNetworkCounter(wifi, L"\\Network Interface(MediaTek Wi-Fi 6 MT7921 Wireless LAN Card)\\Bytes %s/sec");
    InitializeNetworkCounter(ethernet, L"\\Network Interface(Realtek PCIe GBE Family Controller)\\Bytes %s/sec");

    // Initialize CPU time tracking
    QueryPerformanceCounter(&lastCPUTime);
    lastSysTime = lastCPUTime;
}

StatsV1::~StatsV1() noexcept {
    if (cpuQuery) {
        if (counterPerf) PdhRemoveCounter(counterPerf);
        if (counterFreq) PdhRemoveCounter(counterFreq);
        PdhCloseQuery(cpuQuery);
    }

    CleanupNetworkCounter(wifi);
    CleanupNetworkCounter(ethernet);
}

bool StatsV1::InitializeNetworkCounter(NetworkCounters& counter, const wchar_t* interfaceNameFormat) {
    if (PdhOpenQuery(nullptr, 0, &counter.query) != ERROR_SUCCESS) {
        return false;
    }

    wchar_t sendPath[256];
    wchar_t receivePath[256];
    swprintf_s(sendPath, interfaceNameFormat, L"Sent");
    swprintf_s(receivePath, interfaceNameFormat, L"Received");

    if (PdhAddCounterW(counter.query, sendPath, 0, &counter.sendCounter) != ERROR_SUCCESS ||
        PdhAddCounterW(counter.query, receivePath, 0, &counter.receiveCounter) != ERROR_SUCCESS) {
        CleanupNetworkCounter(counter);
        return false;
    }

    // Initialize with first collection
    PdhCollectQueryData(counter.query);
    return true;
}

void StatsV1::CleanupNetworkCounter(NetworkCounters& counter) noexcept {
    if (counter.query) {
        if (counter.sendCounter) PdhRemoveCounter(counter.sendCounter);
        if (counter.receiveCounter) PdhRemoveCounter(counter.receiveCounter);
        PdhCloseQuery(counter.query);
        counter.query = nullptr;
    }
}

float StatsV1::GetNetworkRate(PDH_HQUERY query, PDH_HCOUNTER counter) noexcept {
    if (!query || !counter) return 0.0f;

    PDH_FMT_COUNTERVALUE counterVal;
    if (PdhCollectQueryData(query) != ERROR_SUCCESS) return 0.0f;
    
    if (PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &counterVal) != ERROR_SUCCESS) {
        return 0.0f;
    }

    return static_cast<float>(counterVal.doubleValue * BYTES_TO_MBPS);
}

float StatsV1::GETCPUFrequency() noexcept {
    if (!cpuQuery || !counterFreq) return CPUFrequency;
    if (!cpuQuery || !counterPerf) return CPUFPerf;

    for (int i = 0; i < 3; i++) {
        PdhCollectQueryData(cpuQuery);
        Sleep(100);
    }

    if (PdhCollectQueryData(cpuQuery) != ERROR_SUCCESS) {
        return CPUFrequency;
    }
    Sleep(100);
    PDH_FMT_COUNTERVALUE perfValue, freqValue;
    if (PdhGetFormattedCounterValue(counterPerf, PDH_FMT_LONG, NULL, &perfValue) != ERROR_SUCCESS) {
        return CPUFrequency;
    }
    if (PdhGetFormattedCounterValue(counterFreq, PDH_FMT_LONG, NULL, &freqValue) != ERROR_SUCCESS) {
        return CPUFrequency;
    }

    CPUFrequency = (perfValue.longValue / 100.0) * freqValue.longValue;
        
    return CPUFrequency;
}


float StatsV1::GETCPUtilization() noexcept {
    FILETIME idleTime, kernelTime, userTime;
    FILETIME newIdleTime, newKernelTime, newUserTime;

    // Get initial CPU times
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return 0.0f; // If retrieval fails, return 0 utilization
    }

    Sleep(MEASUREMENT_DELAY); // Wait for measurement interval

    // Get CPU times after delay
    if (!GetSystemTimes(&newIdleTime, &newKernelTime, &newUserTime)) {
        return 0.0f; // Return 0 on failure
    }

    // Convert FILETIME to ULONGLONG
    auto fileTimeToUint64 = [](const FILETIME& ft) -> ULONGLONG {
        return (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    };

    ULONGLONG idle = fileTimeToUint64(newIdleTime) - fileTimeToUint64(idleTime);
    ULONGLONG kernel = fileTimeToUint64(newKernelTime) - fileTimeToUint64(kernelTime);
    ULONGLONG user = fileTimeToUint64(newUserTime) - fileTimeToUint64(userTime);
    
    ULONGLONG total = kernel + user;
    CPUUtilization = 0.0f;

    if (total > 0) {
        CPUUtilization = static_cast<float>(total - idle) * 100.0f / total;
    }

    return CPUUtilization;
}

float StatsV1::GETRAMUsed() noexcept {
    if (GlobalMemoryStatusEx(&memInfo)) {
        return static_cast<float>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / BYTES_TO_GB;
    }
    return 0.0f;
}

float StatsV1::GETRAMUtilization() noexcept {
    float used = GETRAMUsed();
    return RAMTotal > 0.0f ? (used / RAMTotal) * 100.0f : 0.0f;
}

float StatsV1::GETDISKUsed() noexcept {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
    if (GetDiskFreeSpaceExW(L"C:\\", &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        return static_cast<float>(totalBytes.QuadPart - totalFreeBytes.QuadPart) / BYTES_TO_GB;
    }
    return 0.0f;
}

float StatsV1::GETDISKUtilization() noexcept {
    float used = GETDISKUsed();
    return DISKTotal > 0.0f ? (used / DISKTotal) * 100.0f : 0.0f;
}

float StatsV1::GETWiFiSend() noexcept {
    return GetNetworkRate(wifi.query, wifi.sendCounter);
}

float StatsV1::GETWiFiReceive() noexcept {
    return GetNetworkRate(wifi.query, wifi.receiveCounter);
}

float StatsV1::GETEthernetSend() noexcept {
    return GetNetworkRate(ethernet.query, ethernet.sendCounter);
}

float StatsV1::GETEthernetReceive() noexcept {
    return GetNetworkRate(ethernet.query, ethernet.receiveCounter);
}
