#include <windows.h>
#include <iphlpapi.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "iphlpapi.lib")

void ListNetworkAdaptersWithGUIDs() {
    // Buffer to store the adapter information
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        std::cerr << "Error allocating memory for adapter info." << std::endl;
        return;
    }

    // Get adapter information
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            std::cerr << "Error allocating memory for adapter info." << std::endl;
            return;
        }
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            std::wcout << L"Adapter Name: " << pAdapter->AdapterName << std::endl;
            std::wcout << L"Adapter Description: " << pAdapter->Description << std::endl;
            std::wcout << L"Adapter GUID: " << pAdapter->AdapterName << std::endl;
            std::wcout << L"-------------------------------------------" << std::endl;

            pAdapter = pAdapter->Next;
        }
    }

    if (pAdapterInfo) {
        free(pAdapterInfo);
    }
}

int main() {
    ListNetworkAdaptersWithGUIDs();
    return 0;
}
