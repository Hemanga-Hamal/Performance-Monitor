#include <windows.h>
#include <pdh.h>
#include <iostream>
#include <thread>

void GetRealTimeCpuFrequency() {
    PDH_HQUERY query = NULL;
    PDH_HCOUNTER counterPerf = NULL;
    PDH_HCOUNTER counterFreq = NULL;
    PDH_STATUS status;

    // Open a query
    status = PdhOpenQuery(NULL, 0, &query);
    if (status != ERROR_SUCCESS) {
        std::cerr << "Failed to open query: " << status << std::endl;
        return;
    }

    // Add counters
    status = PdhAddCounterW(query, L"\\Processor Information(_Total)\\% Processor Performance", 0, &counterPerf);
    if (status != ERROR_SUCCESS) {
        std::cerr << "Failed to add performance counter: " << status << std::endl;
        PdhCloseQuery(query);
        return;
    }

    status = PdhAddCounterW(query, L"\\Processor Information(_Total)\\Processor Frequency", 0, &counterFreq);
    if (status != ERROR_SUCCESS) {
        std::cerr << "Failed to add frequency counter: " << status << std::endl;
        PdhCloseQuery(query);
        return;
    }

    try {
        // Initial collection cycles
        for (int i = 0; i < 3; i++) {
            PdhCollectQueryData(query);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        while (true) {
            status = PdhCollectQueryData(query);
            if (status != ERROR_SUCCESS) {
                std::cerr << "Failed to collect data: " << status << std::endl;
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Get values
            PDH_FMT_COUNTERVALUE perfValue, freqValue;
            
            status = PdhGetFormattedCounterValue(counterPerf, PDH_FMT_LONG, NULL, &perfValue);
            if (status != ERROR_SUCCESS) {
                std::cerr << "Failed to get performance value: " << status << std::endl;
                continue;
            }
            
            status = PdhGetFormattedCounterValue(counterFreq, PDH_FMT_LONG, NULL, &freqValue);
            if (status != ERROR_SUCCESS) {
                std::cerr << "Failed to get frequency value: " << status << std::endl;
                continue;
            }

            // Calculate real-time frequency
            double realTimeFrequency = (perfValue.longValue / 100.0) * freqValue.longValue;
            
            std::cout << "Estimated Real-Time CPU Frequency: " << realTimeFrequency << " MHz" << std::endl;
        }
    }
    catch (...) {
        std::cerr << "An exception occurred" << std::endl;
    }

    // Cleanup
    PdhCloseQuery(query);
}

int main() {
    try {
        GetRealTimeCpuFrequency();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}