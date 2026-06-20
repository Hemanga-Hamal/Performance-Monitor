#include "StatsV1.h"
#include <string>
#include <pdh.h>
#include <vector>

namespace {
    constexpr float BYTES_TO_GB = 1024.0f * 1024.0f * 1024.0f;
    constexpr float BYTES_TO_MBPS = 8.0f / 1e6;

    std::vector<std::wstring> FindNetworkAdapters() {
        std::vector<std::wstring> adapters;
        DWORD instBufSize = 0;
        DWORD counterBufSize = 0;
        PdhEnumObjectItemsW(nullptr, nullptr, L"Network Interface",
                            nullptr, &instBufSize, nullptr, &counterBufSize,
                            PERF_DETAIL_WIZARD, 0);
        if (instBufSize == 0) return adapters;

        std::vector<wchar_t> instances(instBufSize);
        if (PdhEnumObjectItemsW(nullptr, nullptr, L"Network Interface",
                                instances.data(), &instBufSize, nullptr, &counterBufSize,
                                PERF_DETAIL_WIZARD, 0) != ERROR_SUCCESS) {
            return adapters;
        }

        for (const wchar_t* inst = instances.data(); *inst; inst += wcslen(inst) + 1) {
            adapters.push_back(inst);
        }
        return adapters;
    }

    bool NameContainsKeyword(const std::wstring& name, const wchar_t* keyword) {
        std::wstring lowerName = name;
        std::wstring lowerKeyword = keyword;
        for (auto& c : lowerName) c = towlower(c);
        for (auto& c : lowerKeyword) c = towlower(c);
        return lowerName.find(lowerKeyword) != std::wstring::npos;
    }
}

StatsV1::StatsV1() noexcept {
    if (PdhOpenQuery(nullptr, 0, &cpuQuery) == ERROR_SUCCESS) {
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
        if (cpuQuery) {
            PdhCollectQueryData(cpuQuery);
        }
    }

    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        RAMTotal = static_cast<float>(memInfo.ullTotalPhys) / BYTES_TO_GB;
    }

    // Read CPU model from registry
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t buf[256];
        DWORD bufSize = sizeof(buf);
        if (RegQueryValueExW(hKey, L"ProcessorNameString", nullptr, nullptr,
                             reinterpret_cast<LPBYTE>(buf), &bufSize) == ERROR_SUCCESS) {
            int len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
            cpuModel.resize(len > 0 ? len - 1 : 0);
            if (len > 0) WideCharToMultiByte(CP_UTF8, 0, buf, -1, &cpuModel[0], len, nullptr, nullptr);
        }
        RegCloseKey(hKey);
    }

    // Enumerate all fixed disks
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26; i++) {
        if (!(drives & (1 << i))) continue;
        wchar_t root[4] = {static_cast<wchar_t>(L'A' + i), L':', L'\\', L'\0'};
        if (GetDriveTypeW(root) != DRIVE_FIXED) continue;

        ULARGE_INTEGER totalBytes, freeBytes;
        if (GetDiskFreeSpaceExW(root, nullptr, &totalBytes, &freeBytes)) {
            DiskInfo disk;
            disk.name = root;
            disk.totalGB = static_cast<float>(totalBytes.QuadPart) / BYTES_TO_GB;
            disks.push_back(disk);
        }
    }

    // Dynamic network adapter discovery
    std::vector<std::wstring> adapters = FindNetworkAdapters();
    std::wstring wifiName, ethName;
    for (const auto& name : adapters) {
        if (wifiName.empty() && (NameContainsKeyword(name, L"wi-fi") ||
                                  NameContainsKeyword(name, L"wifi") ||
                                  NameContainsKeyword(name, L"wireless") ||
                                  NameContainsKeyword(name, L"wlan") ||
                                  NameContainsKeyword(name, L"802.11"))) {
            wifiName = name;
        } else if (ethName.empty() && !NameContainsKeyword(name, L"bluetooth") &&
                    !NameContainsKeyword(name, L"virtual") &&
                    !NameContainsKeyword(name, L"loopback") &&
                    !NameContainsKeyword(name, L"teredo") &&
                    !NameContainsKeyword(name, L"isatap")) {
            ethName = name;
        }
    }

    // Fallback: if only one adapter found, assign it to ethernet
    if (adapters.size() == 1 && wifiName.empty() && ethName.empty()) {
        ethName = adapters[0];
    }
    if (!wifiName.empty()) {
        InitializeNetworkCounter(wifi, wifiName.c_str());
    }
    if (!ethName.empty()) {
        InitializeNetworkCounter(ethernet, ethName.c_str());
    }

    // Save for diagnostics
    discoveredAdapters = std::move(adapters);

    // Initialize GPU query
    if (PdhOpenQuery(nullptr, 0, &gpuQuery) == ERROR_SUCCESS) {
        DWORD bufSize = 0;
        DWORD counterBufSize = 0;
        PdhEnumObjectItemsW(nullptr, nullptr, L"GPU Engine",
                            nullptr, &bufSize, nullptr, &counterBufSize,
                            PERF_DETAIL_WIZARD, 0);
        if (bufSize > 0) {
            std::vector<wchar_t> instances(bufSize);
            if (PdhEnumObjectItemsW(nullptr, nullptr, L"GPU Engine",
                                    instances.data(), &bufSize, nullptr, &counterBufSize,
                                    PERF_DETAIL_WIZARD, 0) == ERROR_SUCCESS) {
                for (const wchar_t* inst = instances.data(); *inst; inst += wcslen(inst) + 1) {
                    if (wcscmp(inst, L"_Total") == 0) continue;
                    wchar_t gpuPath[256];
                    swprintf_s(gpuPath, L"\\GPU Engine(%s)\\Utilization Percentage", inst);
                    if (PdhAddCounterW(gpuQuery, gpuPath, 0, &gpuCounter) == ERROR_SUCCESS) {
                        gpuName = inst;
                        PdhCollectQueryData(gpuQuery);
                        break;
                    }
                }
            }
        }
        if (gpuName.empty()) {
            PdhCloseQuery(gpuQuery);
            gpuQuery = nullptr;
        }
    }

    // Read GPU model name via EnumDisplayDevices
    DISPLAY_DEVICEW dd = {sizeof(dd)};
    if (EnumDisplayDevicesW(nullptr, 0, &dd, 0)) {
        int len = WideCharToMultiByte(CP_UTF8, 0, dd.DeviceString, -1, nullptr, 0, nullptr, nullptr);
        gpuModel.resize(len > 0 ? len - 1 : 0);
        if (len > 0) WideCharToMultiByte(CP_UTF8, 0, dd.DeviceString, -1, &gpuModel[0], len, nullptr, nullptr);
    }

    QueryPerformanceCounter(&lastCPUTime);
}

StatsV1::~StatsV1() noexcept {
    if (cpuQuery) {
        if (counterPerf) PdhRemoveCounter(counterPerf);
        if (counterFreq) PdhRemoveCounter(counterFreq);
        PdhCloseQuery(cpuQuery);
    }
    CleanupNetworkCounter(wifi);
    CleanupNetworkCounter(ethernet);

    if (gpuQuery) {
        if (gpuCounter) PdhRemoveCounter(gpuCounter);
        PdhCloseQuery(gpuQuery);
    }
}

bool StatsV1::InitializeNetworkCounter(NetworkCounters& counter, const wchar_t* adapterName) {
    if (PdhOpenQuery(nullptr, 0, &counter.query) != ERROR_SUCCESS) {
        return false;
    }

    wchar_t sendPath[PDH_MAX_COUNTER_PATH];
    wchar_t receivePath[PDH_MAX_COUNTER_PATH];
    swprintf_s(sendPath, L"\\Network Interface(%s)\\Bytes Sent/sec", adapterName);
    swprintf_s(receivePath, L"\\Network Interface(%s)\\Bytes Received/sec", adapterName);

    if (PdhAddCounterW(counter.query, sendPath, 0, &counter.sendCounter) != ERROR_SUCCESS ||
        PdhAddCounterW(counter.query, receivePath, 0, &counter.receiveCounter) != ERROR_SUCCESS) {
        CleanupNetworkCounter(counter);
        return false;
    }

    PdhCollectQueryData(counter.query);
    QueryPerformanceCounter(&counter.lastCollect);
    counter.primed = true;
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

float StatsV1::GetNetworkRate(NetworkCounters& counter, PDH_HCOUNTER hCounter) noexcept {
    if (!counter.query || !hCounter || !counter.primed) return 0.0f;

    LARGE_INTEGER now, freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);
    double elapsed = static_cast<double>(now.QuadPart - counter.lastCollect.QuadPart) / freq.QuadPart;
    if (elapsed < 0.5) return counter.sendRate;

    if (PdhCollectQueryData(counter.query) != ERROR_SUCCESS) {
        counter.sendRate = 0.0f;
        counter.receiveRate = 0.0f;
        return 0.0f;
    }
    counter.lastCollect = now;

    PDH_FMT_COUNTERVALUE val;
    if (PdhGetFormattedCounterValue(counter.sendCounter, PDH_FMT_DOUBLE, nullptr, &val) == ERROR_SUCCESS) {
        counter.sendRate = static_cast<float>(val.doubleValue * BYTES_TO_MBPS);
    }
    if (PdhGetFormattedCounterValue(counter.receiveCounter, PDH_FMT_DOUBLE, nullptr, &val) == ERROR_SUCCESS) {
        counter.receiveRate = static_cast<float>(val.doubleValue * BYTES_TO_MBPS);
    }

    if (hCounter == counter.sendCounter) return counter.sendRate;
    return counter.receiveRate;
}

float StatsV1::GETCPUFrequency() noexcept {
    if (!cpuQuery || !counterFreq || !counterPerf) return CPUFrequency.load();

    LARGE_INTEGER now, freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);

    if (!freqPrimed) {
        PdhCollectQueryData(cpuQuery);
        freqCollectTime = now;
        freqPrimed = true;
        return CPUFrequency.load();
    }

    double elapsed = static_cast<double>(now.QuadPart - freqCollectTime.QuadPart) / freq.QuadPart;
    if (elapsed < 0.2) return CPUFrequency.load();

    if (PdhCollectQueryData(cpuQuery) != ERROR_SUCCESS) {
        freqPrimed = false;
        return CPUFrequency.load();
    }

    PDH_FMT_COUNTERVALUE perfValue, freqValue;
    if (PdhGetFormattedCounterValue(counterPerf, PDH_FMT_LONG, NULL, &perfValue) != ERROR_SUCCESS ||
        PdhGetFormattedCounterValue(counterFreq, PDH_FMT_LONG, NULL, &freqValue) != ERROR_SUCCESS) {
        freqPrimed = false;
        return CPUFrequency.load();
    }

    CPUFrequency.store((perfValue.longValue / 100.0f) * freqValue.longValue);
    freqCollectTime = now;
    return CPUFrequency.load();
}

float StatsV1::GETCPUtilization() noexcept {
    if (!cpuUtilPrimed) {
        GetSystemTimes(&prevIdleTime, &prevKernelTime, &prevUserTime);
        cpuUtilPrimed = true;
        return CPUUtilization.load();
    }

    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return CPUUtilization.load();
    }

    auto fileTimeToUint64 = [](const FILETIME& ft) -> ULONGLONG {
        return (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
    };

    ULONGLONG idle = fileTimeToUint64(idleTime) - fileTimeToUint64(prevIdleTime);
    ULONGLONG kernel = fileTimeToUint64(kernelTime) - fileTimeToUint64(prevKernelTime);
    ULONGLONG user = fileTimeToUint64(userTime) - fileTimeToUint64(prevUserTime);

    prevIdleTime = idleTime;
    prevKernelTime = kernelTime;
    prevUserTime = userTime;

    ULONGLONG total = kernel + user;
    if (total > 0) {
        CPUUtilization.store(static_cast<float>(total - idle) * 100.0f / total);
    }

    return CPUUtilization.load();
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

const wchar_t* StatsV1::GETDiskName(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(disks.size())) return L"";
    return disks[index].name.c_str();
}

float StatsV1::GETDiskTotal(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(disks.size())) return 0.0f;
    return disks[index].totalGB;
}

float StatsV1::GETDiskUsed(int index) noexcept {
    if (index < 0 || index >= static_cast<int>(disks.size())) return 0.0f;
    const wchar_t* root = disks[index].name.c_str();
    ULARGE_INTEGER totalBytes, freeBytes;
    if (GetDiskFreeSpaceExW(root, nullptr, &totalBytes, &freeBytes)) {
        disks[index].totalGB = static_cast<float>(totalBytes.QuadPart) / BYTES_TO_GB;
        return static_cast<float>(totalBytes.QuadPart - freeBytes.QuadPart) / BYTES_TO_GB;
    }
    return 0.0f;
}

float StatsV1::GETDiskUtilization(int index) noexcept {
    float used = GETDiskUsed(index);
    float total = GETDiskTotal(index);
    return total > 0.0f ? (used / total) * 100.0f : 0.0f;
}

float StatsV1::GETGPUUtilization() noexcept {
    if (!gpuQuery || !gpuCounter) return GPUUtilization.load();

    LARGE_INTEGER now, freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);

    if (!gpuPrimed) {
        PdhCollectQueryData(gpuQuery);
        gpuCollectTime = now;
        gpuPrimed = true;
        return GPUUtilization.load();
    }

    double elapsed = static_cast<double>(now.QuadPart - gpuCollectTime.QuadPart) / freq.QuadPart;
    if (elapsed < 0.5) return GPUUtilization.load();

    PDH_FMT_COUNTERVALUE val;
    if (PdhCollectQueryData(gpuQuery) != ERROR_SUCCESS ||
        PdhGetFormattedCounterValue(gpuCounter, PDH_FMT_DOUBLE, nullptr, &val) != ERROR_SUCCESS) {
        gpuPrimed = false;
        return GPUUtilization.load();
    }

    GPUUtilization.store(static_cast<float>(val.doubleValue));
    gpuCollectTime = now;
    return GPUUtilization.load();
}

float StatsV1::GETWiFiSend() noexcept {
    return GetNetworkRate(wifi, wifi.sendCounter);
}

float StatsV1::GETWiFiReceive() noexcept {
    return GetNetworkRate(wifi, wifi.receiveCounter);
}

float StatsV1::GETEthernetSend() noexcept {
    return GetNetworkRate(ethernet, ethernet.sendCounter);
}

float StatsV1::GETEthernetReceive() noexcept {
    return GetNetworkRate(ethernet, ethernet.receiveCounter);
}
