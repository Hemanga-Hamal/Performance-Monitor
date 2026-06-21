#include "NetworkMonitor.h"

namespace {
    constexpr double MIN_COLLECT_INTERVAL = 0.2;

    std::vector<std::wstring> FindNetworkAdapters() {
        std::vector<std::wstring> seen;
        DWORD bufSize = 0;

        PdhExpandWildCardPathW(nullptr, L"\\Network Interface(*)\\Bytes Total/sec",
                                nullptr, &bufSize, 0);
        if (bufSize == 0) {
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
            for (const auto& s : seen) {
                if (s == name) { dup = true; break; }
            }
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

    void CleanAdapterName(std::wstring& name) {
        size_t pos = std::wstring::npos;
        while ((pos = name.find(L"[R]")) != std::wstring::npos) name.erase(pos, 3);
        while ((pos = name.find(L"(R)")) != std::wstring::npos) name.erase(pos, 3);
        pos = name.find(L" #");
        if (pos != std::wstring::npos) name.erase(pos);
        if (name.size() > 40) name = name.substr(0, 37) + L"...";
    }
}

NetworkMonitor::NetworkMonitor() {
    QueryPerformanceFrequency(&perfFrequency);

    std::vector<std::wstring> adapters = FindNetworkAdapters();
    std::wstring wifiName, ethName;

    for (const auto& name : adapters) {
        if (wifiName.empty() && (NameContainsKeyword(name, L"wi-fi") ||
                                  NameContainsKeyword(name, L"wifi") ||
                                  NameContainsKeyword(name, L"wireless") ||
                                  NameContainsKeyword(name, L"wlan") ||
                                  NameContainsKeyword(name, L"802.11"))) {
            wifiName = name;
        } else if (ethName.empty() &&
                    !NameContainsKeyword(name, L"bluetooth") &&
                    !NameContainsKeyword(name, L"virtual") &&
                    !NameContainsKeyword(name, L"loopback") &&
                    !NameContainsKeyword(name, L"teredo") &&
                    !NameContainsKeyword(name, L"isatap") &&
                    !NameContainsKeyword(name, L"wan miniport")) {
            ethName = name;
        }
    }

    if (adapters.size() == 1 && wifiName.empty() && ethName.empty()) {
        ethName = adapters[0];
    }

    if (!wifiName.empty()) {
        InitializeNetworkCounter(wifi, wifiName.c_str());
    }
    if (!ethName.empty()) {
        InitializeNetworkCounter(ethernet, ethName.c_str());
    }

    discoveredAdapters = std::move(adapters);
    for (const auto& name : discoveredAdapters) {
        AdapterInfo info;
        info.name = name;
        CleanAdapterName(info.name);
        info.isWiFi = (name == wifiName);
        info.isEthernet = (name == ethName);
        adapterInfos.push_back(info);
    }
}

NetworkMonitor::~NetworkMonitor() noexcept {
    CleanupNetworkCounter(wifi);
    CleanupNetworkCounter(ethernet);
}

bool NetworkMonitor::InitializeNetworkCounter(NetworkCounters& nc, const wchar_t* adapterName) {
    if (PdhOpenQuery(nullptr, 0, &nc.query) != ERROR_SUCCESS) {
        return false;
    }

    wchar_t sendPath[PDH_MAX_COUNTER_PATH];
    wchar_t receivePath[PDH_MAX_COUNTER_PATH];
    swprintf_s(sendPath, PDH_MAX_COUNTER_PATH, L"\\Network Interface(%s)\\Bytes Sent/sec", adapterName);
    swprintf_s(receivePath, PDH_MAX_COUNTER_PATH, L"\\Network Interface(%s)\\Bytes Received/sec", adapterName);

    if (PdhAddCounterW(nc.query, sendPath, 0, &nc.sendCounter) != ERROR_SUCCESS ||
        PdhAddCounterW(nc.query, receivePath, 0, &nc.receiveCounter) != ERROR_SUCCESS) {
        CleanupNetworkCounter(nc);
        return false;
    }

    PdhCollectQueryData(nc.query);
    QueryPerformanceCounter(&nc.lastCollect);
    nc.primed = true;
    return true;
}

void NetworkMonitor::CleanupNetworkCounter(NetworkCounters& nc) noexcept {
    if (nc.query) {
        if (nc.sendCounter) PdhRemoveCounter(nc.sendCounter);
        if (nc.receiveCounter) PdhRemoveCounter(nc.receiveCounter);
        PdhCloseQuery(nc.query);
        nc.query = nullptr;
    }
}

float NetworkMonitor::GetNetworkRate(NetworkCounters& nc, PDH_HCOUNTER hCounter) noexcept {
    if (!nc.query || !hCounter || !nc.primed) return 0.0f;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double elapsed = static_cast<double>(now.QuadPart - nc.lastCollect.QuadPart)
                     / perfFrequency.QuadPart;
    if (elapsed < MIN_COLLECT_INTERVAL) {
        return (hCounter == nc.sendCounter) ? nc.sendRate : nc.receiveRate;
    }

    if (PdhCollectQueryData(nc.query) != ERROR_SUCCESS) {
        nc.sendRate = 0.0f;
        nc.receiveRate = 0.0f;
        return 0.0f;
    }
    nc.lastCollect = now;

    PDH_FMT_COUNTERVALUE val;
    if (PdhGetFormattedCounterValue(nc.sendCounter, PDH_FMT_DOUBLE, nullptr, &val) == ERROR_SUCCESS) {
        nc.sendRate = static_cast<float>(val.doubleValue);
    }
    if (PdhGetFormattedCounterValue(nc.receiveCounter, PDH_FMT_DOUBLE, nullptr, &val) == ERROR_SUCCESS) {
        nc.receiveRate = static_cast<float>(val.doubleValue);
    }

    if (hCounter == nc.sendCounter) return nc.sendRate;
    return nc.receiveRate;
}

float NetworkMonitor::GetWiFiSend() noexcept {
    for (const auto& a : adapterInfos) { if (a.isWiFi && !a.enabled) return 0.0f; }
    return GetNetworkRate(wifi, wifi.sendCounter);
}

float NetworkMonitor::GetWiFiReceive() noexcept {
    for (const auto& a : adapterInfos) { if (a.isWiFi && !a.enabled) return 0.0f; }
    return GetNetworkRate(wifi, wifi.receiveCounter);
}

float NetworkMonitor::GetEthernetSend() noexcept {
    for (const auto& a : adapterInfos) { if (a.isEthernet && !a.enabled) return 0.0f; }
    return GetNetworkRate(ethernet, ethernet.sendCounter);
}

float NetworkMonitor::GetEthernetReceive() noexcept {
    for (const auto& a : adapterInfos) { if (a.isEthernet && !a.enabled) return 0.0f; }
    return GetNetworkRate(ethernet, ethernet.receiveCounter);
}

const std::vector<std::wstring>& NetworkMonitor::GetDiscoveredAdapters() const noexcept {
    return discoveredAdapters;
}

const std::vector<NetworkMonitor::AdapterInfo>& NetworkMonitor::GetAdapters() const noexcept {
    return adapterInfos;
}

void NetworkMonitor::SetAdapterEnabled(int index, bool enabled) noexcept {
    if (index >= 0 && index < static_cast<int>(adapterInfos.size())) {
        adapterInfos[index].enabled = enabled;
    }
}
