#include <windows.h>
#include <pdh.h>
#include <iostream>
#include <thread>

struct CpuQuery {
    PDH_HQUERY query = NULL;
    PDH_HCOUNTER counterPerf = NULL;
    PDH_HCOUNTER counterFreq = NULL;
};

bool SetupQuery(CpuQuery& cpuQuery) {
    if (PdhOpenQuery(NULL, 0, &cpuQuery.query) != ERROR_SUCCESS) return false;
    if (PdhAddCounterW(cpuQuery.query, L"\\Processor Information(_Total)\\% Processor Performance", 0, &cpuQuery.counterPerf) != ERROR_SUCCESS) return false;
    if (PdhAddCounterW(cpuQuery.query, L"\\Processor Information(_Total)\\Processor Frequency", 0, &cpuQuery.counterFreq) != ERROR_SUCCESS) return false;
    return true;
}

void GetCpuFrequency(CpuQuery& cpuQuery) {
    for (int i = 0; i < 3; i++) {
        PdhCollectQueryData(cpuQuery.query);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    while (true) {
        if (PdhCollectQueryData(cpuQuery.query) != ERROR_SUCCESS) break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        PDH_FMT_COUNTERVALUE perfValue, freqValue;
        if (PdhGetFormattedCounterValue(cpuQuery.counterPerf, PDH_FMT_LONG, NULL, &perfValue) != ERROR_SUCCESS) continue;
        if (PdhGetFormattedCounterValue(cpuQuery.counterFreq, PDH_FMT_LONG, NULL, &freqValue) != ERROR_SUCCESS) continue;
        
        double realTimeFrequency = (perfValue.longValue / 100.0) * freqValue.longValue;
        std::cout << "Estimated Real-Time CPU Frequency: " << realTimeFrequency << " MHz" << std::endl;
    }
}

void CleanupQuery(CpuQuery& cpuQuery) {
    PdhCloseQuery(cpuQuery.query);
}

int main() {
    CpuQuery cpuQuery;
    if (!SetupQuery(cpuQuery)) {
        std::cerr << "Failed to setup query." << std::endl;
        return 1;
    }
    GetCpuFrequency(cpuQuery);
    CleanupQuery(cpuQuery);
    return 0;
}