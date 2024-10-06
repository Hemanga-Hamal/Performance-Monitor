#include <iostream>
#include <windows.h>

int main() {
    // Get the main drive (usually C:)
    LPCWSTR drive = L"C:\\";

    // Variables to store the information
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

    // Get the disk space information
    if (GetDiskFreeSpaceExW(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        // Calculate used space
        ULONGLONG usedBytes = totalBytes.QuadPart - totalFreeBytes.QuadPart;

        // Output total and used space in GB    
        std::wcout << L"Used space on drive " << drive << L": " 
                   << usedBytes / (1024 * 1024 * 1024) << L" GB" << std::endl;
    } else {
        std::cerr << "Error retrieving disk space information." << std::endl;
    }

    return 0;
}
