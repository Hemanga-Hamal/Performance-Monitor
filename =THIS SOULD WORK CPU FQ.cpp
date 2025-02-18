#include <iostream>
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <conio.h>
#include <iomanip> 

int main(){
    PDH_HQUERY HCPUQuery;
    PDH_HCOUNTER HCPUppCounter, HCPUfqCounter;
    PDH_FMT_COUNTERVALUE CPUppValue, CPUfqValue;
    DWORD dwCounterType;
    double CPUpp = 0.0;
    double CPUfq = 0.0;
    double realCPUfq = 0.0;
    
    // Open query
    if (PdhOpenQuery(NULL, 0, &HCPUQuery) != ERROR_SUCCESS){
        std::cout << "Failed to open query" << std::endl;
        return 1;
    }

        // pp
    if (PdhAddCounterW(HCPUQuery, L"\\Processor(_Total)\\% Processor Performance", 0, &HCPUppCounter) != ERROR_SUCCESS){
        std::cout << "Failed to add counter" << std::endl;
        return 1;
    }
        // fq
    if (PdhAddCounterW(HCPUQuery, L"\\Processor(_Total)\\Processor Frequency", 0, &HCPUfqCounter) != ERROR_SUCCESS){
        std::cout << "Failed to add counter" << std::endl;
        return 1;
    }

    // Collect data
    std::cout << "Press Enter to stop..." << std::endl;

    while (true){
        
        if (PdhCollectQueryData(HCPUQuery) != ERROR_SUCCESS){
            std::cout << "Failed to collect query data" << std::endl;
            return 1;
        }

        // Get data pp
        if (PdhGetFormattedCounterValue(HCPUppCounter, PDH_FMT_DOUBLE, &dwCounterType, &CPUppValue) != ERROR_SUCCESS){
            CPUpp = (CPUppValue.doubleValue);
            std::cout << "Failed to get formatted counter value" << std::endl;
            return 1;
        }

        // Get data fq
        if (PdhGetFormattedCounterValue(HCPUfqCounter, PDH_FMT_DOUBLE, &dwCounterType, &CPUfqValue) != ERROR_SUCCESS){
            CPUfq = (CPUfqValue.doubleValue);
            std::cout << "Failed to get formatted counter value" << std::endl;
            return 1;
        }

        //real CPU fq
        realCPUfq = CPUpp*CPUfq;

        std::cout << "CPU Frequency: " << std::fixed << std::setprecision(2) << realCPUfq << "MHz" << std::endl;
        Sleep(1000);

        // Check for Enter key press
        if (_kbhit() && _getch() == '\r') {
            break;
        }
    }
    
    PdhCloseQuery(HCPUQuery);

    return 0;

}