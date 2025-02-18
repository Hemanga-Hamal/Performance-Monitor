#include "StatsV1.h"
#include <intrin.h>
#include <system_error>
#include <pdh.h>

#ifdef _MSC_VER
#pragma comment(lib, "pdh.lib")
#endif

namespace {
    constexpr float BYTES_TO_GB = 1024.0f * 1024.0f * 1024.0f;
    constexpr float BYTES_TO_KBPS = 8.0f / 1000.0f;
    constexpr DWORD MEASUREMENT_DELAY = 500;
}

StatsV1::StatsV1() noexcept {
    // Initialize CPU frequency query
    if (PdhOpenQuery(nullptr, 0, &cpuQuery) == ERROR_SUCCESS) {
        // Use the correct counter path for CPU frequency
        if (PdhAddCounterW(cpuQuery, L"\\Processor Information(_Total)\\Processor Frequency", 0, &cpuFreqCounter) != ERROR_SUCCESS) {
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
        if (cpuFreqCounter) PdhRemoveCounter(cpuFreqCounter);
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

    return static_cast<float>(counterVal.doubleValue * BYTES_TO_KBPS);
}

float StatsV1::GETCPUFrequency() noexcept {
    if (!cpuQuery || !cpuFreqCounter) return CPUFrequency; // Return last known frequency

    // Collect initial data
    if (PdhCollectQueryData(cpuQuery) != ERROR_SUCCESS) {
        return CPUFrequency; // Return last known frequency
    }

    // Wait briefly to get meaningful data
    Sleep(MEASUREMENT_DELAY);

    // Collect data again
    if (PdhCollectQueryData(cpuQuery) != ERROR_SUCCESS) {
        return CPUFrequency; // Return last known frequency
    }

    PDH_FMT_COUNTERVALUE counterValue;
    if (PdhGetFormattedCounterValue(cpuFreqCounter, PDH_FMT_DOUBLE, nullptr, &counterValue) == ERROR_SUCCESS) {
        // Update and return the new frequency
        CPUFrequency = static_cast<float>(counterValue.doubleValue);
    }

    return CPUFrequency;
}


float StatsV1::GETCPUtilization() noexcept {
    FILETIME idleTime, kernelTime, userTime;
    GetSystemTimes(&idleTime, &kernelTime, &userTime);

    Sleep(MEASUREMENT_DELAY);

    FILETIME newIdleTime, newKernelTime, newUserTime;
    GetSystemTimes(&newIdleTime, &newKernelTime, &newUserTime);

    ULONGLONG idle = (((ULONGLONG)newIdleTime.dwHighDateTime << 32) | newIdleTime.dwLowDateTime) -
                     (((ULONGLONG)idleTime.dwHighDateTime << 32) | idleTime.dwLowDateTime);

    ULONGLONG kernel = (((ULONGLONG)newKernelTime.dwHighDateTime << 32) | newKernelTime.dwLowDateTime) -
                      (((ULONGLONG)kernelTime.dwHighDateTime << 32) | kernelTime.dwLowDateTime);

    ULONGLONG user = (((ULONGLONG)newUserTime.dwHighDateTime << 32) | newUserTime.dwLowDateTime) -
                     (((ULONGLONG)userTime.dwHighDateTime << 32) | userTime.dwLowDateTime);

    ULONGLONG total = kernel + user;
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
