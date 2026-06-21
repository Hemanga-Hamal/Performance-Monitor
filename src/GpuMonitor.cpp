#include "GpuMonitor.h"
#include <pdh.h>
#include <dxgi.h>
#include <wbemidl.h>
#include <comdef.h>
#include <algorithm>

#pragma comment(lib, "wbemuuid.lib")

namespace {
    constexpr double MIN_COLLECT_INTERVAL = 0.2;

    std::wstring ExtractEngineType(const std::wstring& rawName) {
        size_t pos = rawName.find(L"engtype_");
        if (pos != std::wstring::npos) {
            return rawName.substr(pos);
        }
        return rawName;
    }

    std::wstring FormatLuidForPdh(const LUID& luid) {
        wchar_t buf[64];
        swprintf_s(buf, 64, L"luid_0x%08X_0x%08X",
                   static_cast<unsigned int>(luid.HighPart),
                   static_cast<unsigned int>(luid.LowPart));
        return buf;
    }
}

GpuMonitor::GpuMonitor() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    comInitialized = SUCCEEDED(hr);

    // Step 1: Expand GPU Engine wildcard path
    DWORD bufSize = 0;
    PdhExpandWildCardPathW(nullptr, L"\\GPU Engine(*)\\Utilization Percentage",
                           nullptr, &bufSize, 0);
    if (bufSize == 0) {
        return;
    }

    std::vector<wchar_t> buf(bufSize + 1);
    if (PdhExpandWildCardPathW(nullptr, L"\\GPU Engine(*)\\Utilization Percentage",
                               buf.data(), &bufSize, 0) != ERROR_SUCCESS) {
        return;
    }

    // Step 2: Parse and deduplicate engine instance names
    std::vector<std::wstring> uniqueEngines;
    for (const wchar_t* p = buf.data(); *p; p += wcslen(p) + 1) {
        std::wstring path = p;
        size_t open = path.find(L'(');
        size_t close = path.find(L')');
        if (open == std::wstring::npos || close == std::wstring::npos) continue;
        std::wstring name = path.substr(open + 1, close - open - 1);
        if (name.empty()) continue;
        if (name == L"_Total") continue;

        std::wstring engineType = ExtractEngineType(name);
        bool dup = false;
        for (const auto& s : uniqueEngines) {
            if (s == engineType) { dup = true; break; }
        }
        if (!dup) uniqueEngines.push_back(engineType);
    }

    if (uniqueEngines.empty()) return;

    // Step 3: Create PDH queries for each unique engine (utilization)
    for (const auto& engineName : uniqueEngines) {
        GpuInfo info;
        info.engineName = engineName;

        if (PdhOpenQuery(nullptr, 0, &info.query) == ERROR_SUCCESS) {
            wchar_t gpuPath[256];
            swprintf_s(gpuPath, 256, L"\\GPU Engine(%s)\\Utilization Percentage",
                       engineName.c_str());
            if (PdhAddCounterW(info.query, gpuPath, 0, &info.counter) != ERROR_SUCCESS) {
                PdhCloseQuery(info.query);
                info.query = nullptr;
            }
        }
        if (info.query) {
            PdhCollectQueryData(info.query);
            QueryPerformanceCounter(&info.collectTime);
            info.primed = true;
        }

        gpus.push_back(std::move(info));
    }

    if (gpus.empty()) return;

    // Step 4: DXGI enumeration to get display name, VRAM total, and LUID
    struct AdapterMeta {
        std::string displayName;
        uint64_t vramTotalMB{0};
        LUID luid{};
        bool valid{false};
    };
    std::vector<AdapterMeta> adapters;

    IDXGIFactory* pFactory = nullptr;
    if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory),
                                     reinterpret_cast<void**>(&pFactory))) && pFactory) {
        IDXGIAdapter* pAdapter = nullptr;
        for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
            if (!pAdapter) continue;

            DXGI_ADAPTER_DESC desc;
            if (SUCCEEDED(pAdapter->GetDesc(&desc))) {
                AdapterMeta ad;
                ad.vramTotalMB = desc.DedicatedVideoMemory / (1024ULL * 1024ULL);
                ad.luid = desc.AdapterLuid;
                ad.valid = true;

                int len = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1,
                                              nullptr, 0, nullptr, nullptr);
                ad.displayName.resize(len > 0 ? len - 1 : 0);
                if (len > 0) {
                    WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1,
                                        &ad.displayName[0], len, nullptr, nullptr);
                }
                adapters.push_back(ad);
            }
            pAdapter->Release();
        }
        pFactory->Release();
    }

    // Step 5: Map DXGI adapter info to GpuInfo entries by index
    if (!adapters.empty()) {
        for (size_t i = 0; i < gpus.size(); i++) {
            size_t adIdx = (i < adapters.size()) ? i : 0;
            gpus[i].displayName = adapters[adIdx].displayName;
            gpus[i].vramTotalMB = adapters[adIdx].vramTotalMB;
        }
    }

    // Step 6: Expand GPU Adapter Memory wildcard for VRAM
    bufSize = 0;
    PdhExpandWildCardPathW(nullptr, L"\\GPU Adapter Memory(*)\\Dedicated Usage",
                           nullptr, &bufSize, 0);
    if (bufSize > 0) {
        std::vector<wchar_t> vramBuf(bufSize + 1);
        if (PdhExpandWildCardPathW(nullptr, L"\\GPU Adapter Memory(*)\\Dedicated Usage",
                                   vramBuf.data(), &bufSize, 0) == ERROR_SUCCESS) {

            // For each GpuInfo, try to match a VRAM counter by DXGI adapter LUID
            for (size_t i = 0; i < gpus.size() && i < adapters.size(); i++) {
                if (!adapters[i].valid) continue;

                std::wstring luidStr = FormatLuidForPdh(adapters[i].luid);
                bool found = false;

                for (const wchar_t* p = vramBuf.data(); *p; p += wcslen(p) + 1) {
                    std::wstring vramPath = p;
                    if (vramPath.find(luidStr) != std::wstring::npos) {
                        if (PdhOpenQuery(nullptr, 0, &gpus[i].memQuery) == ERROR_SUCCESS) {
                            if (PdhAddCounterW(gpus[i].memQuery, vramPath.c_str(), 0,
                                               &gpus[i].memCounter) == ERROR_SUCCESS) {
                                PdhCollectQueryData(gpus[i].memQuery);
                                found = true;
                            } else {
                                PdhCloseQuery(gpus[i].memQuery);
                                gpus[i].memQuery = nullptr;
                            }
                        }
                        break;
                    }
                }

                // Fallback: use the first available VRAM counter if no LUID match
                if (!found) {
                    const wchar_t* firstPath = vramBuf.data();
                    if (*firstPath) {
                        if (PdhOpenQuery(nullptr, 0, &gpus[i].memQuery) == ERROR_SUCCESS) {
                            if (PdhAddCounterW(gpus[i].memQuery, firstPath, 0,
                                               &gpus[i].memCounter) != ERROR_SUCCESS) {
                                PdhCloseQuery(gpus[i].memQuery);
                                gpus[i].memQuery = nullptr;
                            } else {
                                PdhCollectQueryData(gpus[i].memQuery);
                            }
                        }
                    }
                }
            }
        }
    }

    // Step 7: WMI query for GPU clock speed
    IWbemLocator* pLoc = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, reinterpret_cast<void**>(&pLoc));
    if (SUCCEEDED(hr) && pLoc) {
        IWbemServices* pSvc = nullptr;
        hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr,
                                  0, nullptr, nullptr, &pSvc);
        if (SUCCEEDED(hr) && pSvc) {
            hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                                    nullptr, RPC_C_AUTHN_LEVEL_CALL,
                                    RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
            if (SUCCEEDED(hr)) {
                IEnumWbemClassObject* pEnum = nullptr;
                hr = pSvc->ExecQuery(
                    _bstr_t(L"WQL"),
                    _bstr_t(L"SELECT CurrentClockSpeed FROM Win32_VideoController"),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    nullptr, &pEnum);
                if (SUCCEEDED(hr) && pEnum) {
                    IWbemClassObject* pObj = nullptr;
                    ULONG uReturn = 0;
                    int adapterIdx = 0;
                    while (pEnum) {
                        hr = pEnum->Next(WBEM_INFINITE, 1, &pObj, &uReturn);
                        if (FAILED(hr) || uReturn == 0) break;

                        VARIANT vtProp;
                        VariantInit(&vtProp);
                        hr = pObj->Get(L"CurrentClockSpeed", 0, &vtProp, nullptr, nullptr);
                        if (SUCCEEDED(hr) && vtProp.vt == VT_I4) {
                            uint32_t clock = static_cast<uint32_t>(vtProp.lVal);
                            // Map WMI rows to GpuInfo entries by adapter index
                            if (adapterIdx < static_cast<int>(gpus.size())) {
                                gpus[adapterIdx].clockMHz = clock;
                            }
                        }
                        VariantClear(&vtProp);
                        pObj->Release();
                        adapterIdx++;
                    }
                    pEnum->Release();
                }
            }
            pSvc->Release();
        }
        pLoc->Release();
    }
}

GpuMonitor::~GpuMonitor() noexcept {
    for (auto& gpu : gpus) {
        if (gpu.query) {
            if (gpu.counter) PdhRemoveCounter(gpu.counter);
            PdhCloseQuery(gpu.query);
        }
        if (gpu.memQuery) {
            if (gpu.memCounter) PdhRemoveCounter(gpu.memCounter);
            PdhCloseQuery(gpu.memQuery);
        }
    }
    if (comInitialized) {
        CoUninitialize();
    }
}

int GpuMonitor::GetCount() const noexcept {
    return static_cast<int>(gpus.size());
}

float GpuMonitor::GetUtilization(int index) noexcept {
    if (index < 0 || index >= static_cast<int>(gpus.size())) return 0.0f;
    auto& gpu = gpus[index];
    if (!gpu.query || !gpu.counter) return gpu.utilization;

    LARGE_INTEGER now, freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);

    if (!gpu.primed) {
        PdhCollectQueryData(gpu.query);
        gpu.collectTime = now;
        gpu.primed = true;
        return gpu.utilization;
    }

    double elapsed = static_cast<double>(now.QuadPart - gpu.collectTime.QuadPart)
                     / freq.QuadPart;
    if (elapsed < MIN_COLLECT_INTERVAL) return gpu.utilization;

    PDH_FMT_COUNTERVALUE val;
    if (PdhCollectQueryData(gpu.query) != ERROR_SUCCESS ||
        PdhGetFormattedCounterValue(gpu.counter, PDH_FMT_DOUBLE, nullptr, &val) != ERROR_SUCCESS) {
        gpu.primed = false;
        return gpu.utilization;
    }

    gpu.utilization = static_cast<float>(val.doubleValue);
    gpu.collectTime = now;
    return gpu.utilization;
}

float GpuMonitor::GetVramUsedPercent(int index) noexcept {
    if (index < 0 || index >= static_cast<int>(gpus.size())) return 0.0f;
    auto& gpu = gpus[index];
    if (!gpu.memQuery || !gpu.memCounter) return gpu.vramUsedPercent;

    LARGE_INTEGER now, freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);

    double elapsed = static_cast<double>(now.QuadPart - gpu.collectTime.QuadPart)
                     / freq.QuadPart;
    if (elapsed < MIN_COLLECT_INTERVAL) return gpu.vramUsedPercent;

    PDH_FMT_COUNTERVALUE val;
    if (PdhCollectQueryData(gpu.memQuery) != ERROR_SUCCESS ||
        PdhGetFormattedCounterValue(gpu.memCounter, PDH_FMT_LARGE, nullptr, &val) != ERROR_SUCCESS) {
        return gpu.vramUsedPercent;
    }

    uint64_t usedBytes = static_cast<uint64_t>(val.largeValue);
    if (gpu.vramTotalMB > 0) {
        gpu.vramUsedPercent = static_cast<float>(
            static_cast<double>(usedBytes) / (gpu.vramTotalMB * 1024.0 * 1024.0) * 100.0);
    } else {
        gpu.vramUsedPercent = 0.0f;
    }

    gpu.collectTime = now;
    return gpu.vramUsedPercent;
}

const char* GpuMonitor::GetDisplayName(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(gpus.size())) return "";
    return gpus[index].displayName.c_str();
}

const wchar_t* GpuMonitor::GetEngineName(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(gpus.size())) return L"";
    return gpus[index].engineName.c_str();
}

uint64_t GpuMonitor::GetVramTotalMB(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(gpus.size())) return 0;
    return gpus[index].vramTotalMB;
}

uint32_t GpuMonitor::GetClockMHz(int index) const noexcept {
    if (index < 0 || index >= static_cast<int>(gpus.size())) return 0;
    return gpus[index].clockMHz;
}

bool GpuMonitor::IsAvailable() const noexcept {
    return !gpus.empty();
}

const std::vector<GpuMonitor::GpuInfo>& GpuMonitor::GetAll() const noexcept {
    return gpus;
}
