#include "CpuMonitor.h"

CpuMonitor::CpuMonitor() noexcept {
    if (PdhOpenQuery(nullptr, 0, &query) == ERROR_SUCCESS) {
        if (PdhAddCounterW(query, L"\\Processor Information(_Total)\\Processor Frequency", 0, &counterFreq) != ERROR_SUCCESS) {
            PdhCloseQuery(query);
            query = nullptr;
        } else {
            if (PdhAddCounterW(query, L"\\Processor Information(_Total)\\% Processor Performance", 0, &counterPerf) != ERROR_SUCCESS) {
                if (PdhAddCounterW(query, L"\\Processor(_Total)\\% Processor Performance", 0, &counterPerf) != ERROR_SUCCESS) {
                    PdhRemoveCounter(counterFreq);
                    counterFreq = nullptr;
                    PdhCloseQuery(query);
                    query = nullptr;
                }
            }
        }
        if (query) {
            PdhCollectQueryData(query);
        }
    }

    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t buf[256];
        DWORD bufSize = sizeof(buf);
        if (RegQueryValueExW(hKey, L"ProcessorNameString", nullptr, nullptr,
                             reinterpret_cast<LPBYTE>(buf), &bufSize) == ERROR_SUCCESS) {
            int len = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
            model.resize(len > 0 ? len - 1 : 0);
            if (len > 0) WideCharToMultiByte(CP_UTF8, 0, buf, -1, &model[0], len, nullptr, nullptr);
        }

        DWORD mhz = 0;
        DWORD dataSize = sizeof(mhz);
        if (RegQueryValueExW(hKey, L"~MHz", nullptr, nullptr,
                             reinterpret_cast<LPBYTE>(&mhz), &dataSize) == ERROR_SUCCESS) {
            frequency.store(static_cast<float>(mhz));
        }

        RegCloseKey(hKey);
    }
}

CpuMonitor::~CpuMonitor() noexcept {
    if (query) {
        if (counterPerf) PdhRemoveCounter(counterPerf);
        if (counterFreq) PdhRemoveCounter(counterFreq);
        PdhCloseQuery(query);
    }
}

float CpuMonitor::GetFrequency() noexcept {
    if (!query || !counterFreq || !counterPerf) return frequency.load();

    LARGE_INTEGER now, qpf;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&qpf);

    if (!freqPrimed) {
        PdhCollectQueryData(query);
        freqPrimed = true;

        PDH_FMT_COUNTERVALUE perfValue, freqValue;
        if (PdhGetFormattedCounterValue(counterPerf, PDH_FMT_LONG, NULL, &perfValue) == ERROR_SUCCESS) {
            float base = 0.0f;
            if (PdhGetFormattedCounterValue(counterFreq, PDH_FMT_LONG, NULL, &freqValue) == ERROR_SUCCESS) {
                base = static_cast<float>(freqValue.longValue);
            }
            float computed = (perfValue.longValue / 100.0f) * base;
            if (computed > 0.0f) {
                frequency.store(computed);
                freqCollectTime = now;
                return frequency.load();
            }
        }
        freqCollectTime = now;
        return frequency.load();
    }

    double elapsed = static_cast<double>(now.QuadPart - freqCollectTime.QuadPart) / qpf.QuadPart;
    if (elapsed < 0.2) return frequency.load();

    if (PdhCollectQueryData(query) != ERROR_SUCCESS) {
        freqPrimed = false;
        return frequency.load();
    }

    PDH_FMT_COUNTERVALUE perfValue, freqValue;
    if (PdhGetFormattedCounterValue(counterPerf, PDH_FMT_LONG, NULL, &perfValue) != ERROR_SUCCESS ||
        PdhGetFormattedCounterValue(counterFreq, PDH_FMT_LONG, NULL, &freqValue) != ERROR_SUCCESS) {
        freqPrimed = false;
        return frequency.load();
    }

    frequency.store((perfValue.longValue / 100.0f) * freqValue.longValue);
    freqCollectTime = now;
    return frequency.load();
}

float CpuMonitor::GetUtilization() noexcept {
    if (!utilPrimed) {
        GetSystemTimes(&prevIdleTime, &prevKernelTime, &prevUserTime);
        utilPrimed = true;
        return utilization.load();
    }

    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return utilization.load();
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
        utilization.store(static_cast<float>(total - idle) * 100.0f / total);
    }

    return utilization.load();
}

const char* CpuMonitor::GetModel() const noexcept {
    return model.c_str();
}
