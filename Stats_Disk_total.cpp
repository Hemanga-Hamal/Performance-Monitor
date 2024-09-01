#include <iostream>
#include <windows.h>

int main() {
    // Get the main drive (usually C:)
    LPCWSTR drive = L"C:\\";

    // Variables to store the information
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

    // Get the disk space information using the Unicode version of the function
    if (GetDiskFreeSpaceExW(drive, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        std::wcout << L"Total space on drive " << drive << L": " 
                   << totalBytes.QuadPart / (1024 * 1024 * 1024) << L" GB" << std::endl;
    } else {
        std::cerr << "Error retrieving disk space information." << std::endl;
    }

    return 0;
}
