#include "Stats.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <pdh.h>
#include <vector>
#include <algorithm>
#include <dxgi.h>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "wbemuuid.lib")

namespace {
    constexpr float BYTES_TO_GB = 1024.0f * 1024.0f * 1024.0f;
    constexpr float BYTES_TO_MBPS = 8.0f / 1e6;

    std::vector<std::wstring> FindNetworkAdapters() {
        std::vector<std::wstring> seen;
        DWORD bufSize = 0;

        if (PdhExpandWildCardPathW(nullptr, L"\\Network Interface(*)\\Bytes Total/sec",
                                    nullptr, &bufSize, 0) == ERROR_SUCCESS || bufSize == 0) {
            return seen;
        }
        std::vector<wchar_t> buf(bufSize + 1);
        if (PdhExpandWildCardPathW(nullptr, L"\\Network Interface(*)\\Bytes Total/sec",
                                    buf.data(), &bufSize, 0) != ERROR_SUCCESS) {
            return seen;
        }

        for (const wchar_t* p = buf.data(); *p; p += wcslen(p) + 1) {
            std::wstring path = p;
            size_t open = path.find(L'(');
            size_t close = path.find(L')');
            if (open == std::wstring::npos || close == std::wstring::npos) continue;
            std::wstring name = path.substr(open + 1, close - open - 1);
            if (name.empty()) continue;
            bool dup = false;
            for (const auto& s : seen) { if (s == name) { dup = true; break; } }
            if (!dup) seen.push_back(name);
        }
        return seen;
    }

    bool NameContainsKeyword(const std::wstring& name, const wchar_t* keyword) {
        std::wstring lowerName = name;
        std::wstring lowerKeyword = keyword;
        for (auto& c : lowerName) c = towlower(c);
        for (auto& c : lowerKeyword) c = towlower(c);
        return lowerName.find(lowerKeyword) != std::wstring::npos;
    }
}

Stats::Stats() noexcept {
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

    // Save for diagnostics and per-adapter toggles
    discoveredAdapters = std::move(adapters);
    for (const auto& name : discoveredAdapters) {
        AdapterInfo info;
        info.name = name;
        info.isWiFi = (name == wifiName);
        info.isEthernet = (name == ethName);
        adapterInfos.push_back(info);
    }

    // Ã¢â€â‚¬Ã¢â€â‚¬ GPU discovery via DXGI (always works, gives model + total VRAM + LUID) Ã¢â€â‚¬Ã¢â€â‚¬
    {
        IDXGIFactory* dxgiFactory = nullptr;
        if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory))) && dxgiFactory) {
            IDXGIAdapter* adapter = nullptr;
            for (UINT i = 0; dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
                DXGI_ADAPTER_DESC desc;
                if (SUCCEEDED(adapter->GetDesc(&desc))) {
                    GPUInstance gpu;
                    gpu.vramTotalGB = static_cast<float>(desc.DedicatedVideoMemory) / BYTES_TO_GB;

                    // Format LUID string matching PDH/WMI counter instance format
                    // PDH format: luid_<HighPart>_<LowPart>
                    wchar_t luidW[128];
                    swprintf_s(luidW, 128, L"luid_0x%08x_0x%08x",
                               static_cast<unsigned int>(desc.AdapterLuid.HighPart),
                               desc.AdapterLuid.LowPart);
                    for (auto& c : luidW) { if (c >= L'A' && c <= L'F') c += (L'a' - L'A'); }
                    gpu.name = luidW;

                    // Model name
                    int len = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
                    gpu.displayName.resize(len > 0 ? len - 1 : 0);
                    if (len > 0) WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, &gpu.displayName[0], len, nullptr, nullptr);

                    gpuInstances.push_back(std::move(gpu));
                }
                adapter->Release();
            }
            dxgiFactory->Release();
        }
    }

    // Ã¢â€â‚¬Ã¢â€â‚¬ PDH VRAM: match by LUID (same approach as QueryGpuUtilWmi) Ã¢â€â‚¬Ã¢â€â‚¬
    // LUID-based matching is robust regardless of PDH enumeration order or post-sort reordering
    {
        DWORD vramBufSize = 0;
        PdhExpandWildCardPathW(nullptr, L"\\GPU Adapter Memory(*)\\Dedicated Usage",
                                nullptr, &vramBufSize, 0);
        if (vramBufSize > 0) {
            std::vector<wchar_t> vramBuf(vramBufSize + 1);
            if (PdhExpandWildCardPathW(nullptr, L"\\GPU Adapter Memory(*)\\Dedicated Usage",
                                        vramBuf.data(), &vramBufSize, 0) == ERROR_SUCCESS) {
                for (const wchar_t* p = vramBuf.data(); *p; p += wcslen(p) + 1) {
                    std::wstring path = p;
                    size_t open = path.find(L'(');
                    size_t close = path.find(L')');
                    if (open == std::wstring::npos || close == std::wstring::npos) continue;
                    std::wstring name = path.substr(open + 1, close - open - 1);
                    if (name.empty() || name == L"_Total") continue;

                    std::wstring nameLower = name;
                    for (auto& c : nameLower) { if (c >= L'A' && c <= L'F') c += (L'a' - L'A'); }

                    for (auto& gpu : gpuInstances) {
                        if (gpu.vramQuery) continue;
                        std::wstring luidLower = gpu.name;
                        for (auto& c : luidLower) { if (c >= L'A' && c <= L'F') c += (L'a' - L'A'); }
                        if (nameLower.find(luidLower) != std::wstring::npos) {
                            wchar_t vramPath[256];
                            swprintf_s(vramPath, 256, L"\\GPU Adapter Memory(%s)\\Dedicated Usage", name.c_str());
                            if (PdhOpenQuery(nullptr, 0, &gpu.vramQuery) == ERROR_SUCCESS) {
                                if (PdhAddCounterW(gpu.vramQuery, vramPath, 0, &gpu.vramCounter) != ERROR_SUCCESS) {
                                    PdhCloseQuery(gpu.vramQuery);
                                    gpu.vramQuery = nullptr;
                                } else {
                                    PdhCollectQueryData(gpu.vramQuery);
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    // Sort: dedicated GPUs (vramTotal > 0) first
    if (gpuInstances.size() > 1) {
    std::sort(gpuInstances.begin(), gpuInstances.end(),
        [](const GPUInstance& a, const GPUInstance& b) {
            return a.vramTotalGB > b.vramTotalGB;
        });
    }

    // Set global GPU model from first (dedicated) GPU
    if (!gpuInstances.empty()) {
        gpuModel = gpuInstances[0].displayName;
    }
    if (gpuModel.empty()) {
        gpuModel = "GPU";
    }

    // Build LUID list for WMI utilization matching
    for (const auto& gpu : gpuInstances) {
        gpuLuids.push_back(gpu.name);
    }

    // Init WMI (kept alive for periodic GPU util queries)
    InitWbem();

    // GPU clock speed via WMI (uses persistent connection if available)
    if (wbemReady && pWbemSvc) {
        IEnumWbemClassObject* pEnum = nullptr;
        HRESULT hres = pWbemSvc->ExecQuery(bstr_t(L"WQL"),
                                           bstr_t(L"SELECT CurrentClockSpeed FROM Win32_VideoController WHERE Availability=3"),
                                           WBEM_FLAG_FORWARD_ONLY, nullptr, &pEnum);
        if (SUCCEEDED(hres) && pEnum) {
            IWbemClassObject* pObj = nullptr;
            ULONG uReturn = 0;
            int clockIdx = 0;
            while (pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK) {
                VARIANT vtProp;
                VariantInit(&vtProp);
                if (SUCCEEDED(pObj->Get(L"CurrentClockSpeed", 0, &vtProp, 0, 0)) && vtProp.vt == VT_I4) {
                    int clockMHz = vtProp.intVal;
                    if (clockIdx < static_cast<int>(gpuInstances.size())) {
                        gpuInstances[clockIdx].clockSpeedMHz = clockMHz;
                    }
                    clockIdx++;
                }
                VariantClear(&vtProp);
                pObj->Release();
            }
            pEnum->Release();
        }
    }

    QueryPerformanceCounter(&lastCPUTime);
}

Stats::~Stats() noexcept {
    if (cpuQuery) {
        if (counterPerf) PdhRemoveCounter(counterPerf);
        if (counterFreq) PdhRemoveCounter(counterFreq);
        PdhCloseQuery(cpuQuery);
    }
    CleanupNetworkCounter(wifi);
    CleanupNetworkCounter(ethernet);

    for (auto& gpu : gpuInstances) {
        if (gpu.query) {
            if (gpu.counter) PdhRemoveCounter(gpu.counter);
            PdhCloseQuery(gpu.query);
        }
        if (gpu.vramQuery) {
            if (gpu.vramCounter) PdhRemoveCounter(gpu.vramCounter);
            PdhCloseQuery(gpu.vramQuery);
        }
    }

    if (pWbemSvc) pWbemSvc->Release();
    if (pWbemLoc) pWbemLoc->Release();
}

bool Stats::InitializeNetworkCounter(NetworkCounters& counter, const wchar_t* adapterName) {
    if (PdhOpenQuery(nullptr, 0, &counter.query) != ERROR_SUCCESS) {
        return false;
    }

    wchar_t sendPath[PDH_MAX_COUNTER_PATH];
    wchar_t receivePath[PDH_MAX_COUNTER_PATH];
    swprintf_s(sendPath, PDH_MAX_COUNTER_PATH, L"\\Network Interface(%s)\\Bytes Sent/sec", adapterName);
    swprintf_s(receivePath, PDH_MAX_COUNTER_PATH, L"\\Network Interface(%s)\\Bytes Received/sec", adapterName);

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

void Stats::CleanupNetworkCounter(NetworkCounters& counter) noexcept {
    if (counter.query) {
        if (counter.sendCounter) PdhRemoveCounter(counter.sendCounter);
        if (counter.receiveCounter) PdhRemoveCounter(counter.receiveCounter);
        PdhCloseQuery(counter.query);
        counter.query = nullptr;
    }
}

float Stats::GetNetworkRate(NetworkCounters& counter, PDH_HCOUNTER hCounter) noexcept {
    if (!counter.query || !hCounter || !counter.primed) return 0.0f;

    LARGE_INTEGER now, freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);
    double elapsed = static_cast<double>(now.QuadPart - counter.lastCollect.QuadPart) / freq.QuadPart;
    if (elapsed < 0.5) return (hCounter == counter.receiveCounter) ? counter.receiveRate : counter.sendRate;

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

float Stats::GETCPUFrequency() noexcept {
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

float Stats::GETCPUtilization() noexcept {
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

float Stats::GETRAMUsed() noexcept {
    if (GlobalMemoryStatusEx(&memInfo)) {
        return static_cast<float>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / BYTES_TO_GB;
    }
    return 0.0f;
}

float Stats::GETRAMUtilization() noexcept {
    float used = GETRAMUsed();
    return RAMTotal > 0.0f ? (used / RAMTotal) * 100.0f : 0.0f;
}

const wchar_t* Stats::GETDiskName(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(disks.size())) return L"";
    return disks[index].name.c_str();
}

float Stats::GETDiskTotal(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(disks.size())) return 0.0f;
    return disks[index].totalGB;
}

float Stats::GETDiskUsed(int index) noexcept {
    if (index < 0 || index >= static_cast<int>(disks.size())) return 0.0f;
    const wchar_t* root = disks[index].name.c_str();
    ULARGE_INTEGER totalBytes, freeBytes;
    if (GetDiskFreeSpaceExW(root, nullptr, &totalBytes, &freeBytes)) {
        disks[index].totalGB = static_cast<float>(totalBytes.QuadPart) / BYTES_TO_GB;
        return static_cast<float>(totalBytes.QuadPart - freeBytes.QuadPart) / BYTES_TO_GB;
    }
    return 0.0f;
}

float Stats::GETDiskUtilization(int index) noexcept {
    float used = GETDiskUsed(index);
    float total = GETDiskTotal(index);
    return total > 0.0f ? (used / total) * 100.0f : 0.0f;
}

const wchar_t* Stats::GETGPUName(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(gpuInstances.size())) return L"";
    return gpuInstances[index].name.c_str();
}

float Stats::GETGPUUtilization(int index) noexcept {
    if (index < 0 || index >= static_cast<int>(gpuInstances.size())) return 0.0f;

    LARGE_INTEGER now, freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);
    double elapsed = static_cast<double>(now.QuadPart - gpuUtilQueryTime.QuadPart) / freq.QuadPart;
    if (elapsed < 0.5 && index < static_cast<int>(gpuUtilCache.size())) {
        return gpuUtilCache[index];
    }

    if (wbemReady && pWbemSvc) {
        QueryGpuUtilWmi();
        gpuUtilQueryTime = now;
    }

    if (index < static_cast<int>(gpuUtilCache.size())) return gpuUtilCache[index];
    return 0.0f;
}

bool Stats::InitWbem() noexcept {
    HRESULT hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hres) && hres != RPC_E_CHANGED_MODE) return false;

    hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                            IID_IWbemLocator, reinterpret_cast<void**>(&pWbemLoc));
    if (FAILED(hres) || !pWbemLoc) return false;

    hres = pWbemLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr,
                                    0, nullptr, nullptr, &pWbemSvc);
    if (FAILED(hres) || !pWbemSvc) {
        pWbemLoc->Release();
        pWbemLoc = nullptr;
        return false;
    }

    CoSetProxyBlanket(pWbemSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
                      RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    wbemReady = true;
    return true;
}

void Stats::QueryGpuUtilWmi() noexcept {
    if (!pWbemSvc) return;

    IEnumWbemClassObject* pEnum = nullptr;
    HRESULT hres = pWbemSvc->ExecQuery(
        bstr_t(L"WQL"),
        bstr_t(L"SELECT Name, UtilizationPercentage FROM Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine"),
        WBEM_FLAG_FORWARD_ONLY, nullptr, &pEnum);
    if (FAILED(hres) || !pEnum) return;

    gpuUtilCache.assign(gpuInstances.size(), 0.0f);

    IWbemClassObject* pObj = nullptr;
    ULONG uReturn = 0;
    while (pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturn) == S_OK) {
        VARIANT vName, vUtil;
        VariantInit(&vName);
        VariantInit(&vUtil);

        if (SUCCEEDED(pObj->Get(L"Name", 0, &vName, 0, 0)) && vName.vt == VT_BSTR &&
            SUCCEEDED(pObj->Get(L"UtilizationPercentage", 0, &vUtil, 0, 0)) && vUtil.vt == VT_I4) {
            float utilPct = static_cast<float>(vUtil.intVal);
            std::wstring instanceName(vName.bstrVal);
            for (auto& c : instanceName) { if (c >= L'A' && c <= L'F') c += (L'a' - L'A'); }

            for (size_t g = 0; g < gpuLuids.size(); g++) {
                if (!gpuLuids[g].empty() && instanceName.find(gpuLuids[g]) != std::wstring::npos) {
                    gpuUtilCache[g] += utilPct;
                    break;
                }
            }
        }

        VariantClear(&vUtil);
        VariantClear(&vName);
        pObj->Release();
    }
    pEnum->Release();
}

float Stats::GETGPUVRAMUsed(int index) noexcept {
    if (index < 0 || index >= static_cast<int>(gpuInstances.size())) return 0.0f;
    auto& gpu = gpuInstances[index];
    if (!gpu.vramQuery || !gpu.vramCounter) return gpu.cachedVRAMUsed;

    PDH_FMT_COUNTERVALUE val;
    if (PdhCollectQueryData(gpu.vramQuery) == ERROR_SUCCESS &&
        PdhGetFormattedCounterValue(gpu.vramCounter, PDH_FMT_DOUBLE, nullptr, &val) == ERROR_SUCCESS) {
        gpu.cachedVRAMUsed = static_cast<float>(val.doubleValue) / BYTES_TO_GB;
    }
    return gpu.cachedVRAMUsed;
}

float Stats::GETGPUVRAMTotal(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(gpuInstances.size())) return 0.0f;
    return gpuInstances[index].vramTotalGB;
}

int Stats::GETGPUClockSpeed(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(gpuInstances.size())) return 0;
    return gpuInstances[index].clockSpeedMHz;
}

float Stats::GETWiFiSend() noexcept {
    return GetNetworkRate(wifi, wifi.sendCounter);
}

float Stats::GETWiFiReceive() noexcept {
    return GetNetworkRate(wifi, wifi.receiveCounter);
}

float Stats::GETEthernetSend() noexcept {
    return GetNetworkRate(ethernet, ethernet.sendCounter);
}

float Stats::GETEthernetReceive() noexcept {
    return GetNetworkRate(ethernet, ethernet.receiveCounter);
}

void Stats::SetDiskEnabled(int index, bool enabled) noexcept {
    if (index >= 0 && index < static_cast<int>(disks.size())) {
        disks[index].enabled = enabled;
    }
}

void Stats::SetAdapterEnabled(int index, bool enabled) noexcept {
    if (index >= 0 && index < static_cast<int>(adapterInfos.size())) {
        adapterInfos[index].enabled = enabled;
    }
}
