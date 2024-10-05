#include "stats.h"
#include <iostream>

int main() {
    stats systemStats; // Create an instance of the stats class
    int choice;

    do {
        // Display menu options to the user
        std::cout << "\nChoose the system resource to test:" << std::endl;
        std::cout << "1. CPU" << std::endl;
        std::cout << "2. RAM" << std::endl;
        std::cout << "3. DISK" << std::endl;
        std::cout << "4. NETWORK" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Enter your choice (0-4): ";
        std::cin >> choice;

        // Handle invalid input
        while (std::cin.fail() || choice < 0 || choice > 4) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(INT_MAX, '\n'); // Ignore invalid input
            std::cout << "Invalid choice. Please enter a valid option (0-4): ";
            std::cin >> choice;
        }

        switch (choice) {
            case 1:  // CPU
                std::cout << "\n--- Testing CPU ---\n";
                std::cout << "CPU Frequency: " << systemStats.GetCPUFrequency() << " MHz" << std::endl;
                std::cout << "CPU Utilization: " << systemStats.GetCPUUtilization() << " %" << std::endl;
                break;

            case 2:  // RAM
                std::cout << "\n--- Testing RAM ---\n";
                std::cout << "Total RAM: " << systemStats.GETRAMTotal() << " GB" << std::endl;
                std::cout << "Used RAM: " << systemStats.GETRAMUsed() << " GB" << std::endl;
                std::cout << "RAM Utilization: " << systemStats.GETRAMUtilization() << " %" << std::endl;
                break;

            case 3:  // DISK
                std::cout << "\n--- Testing DISK ---\n";
                std::cout << "Total Disk Space: " << systemStats.GETDISKTotal() << " GB" << std::endl;
                std::cout << "Used Disk Space: " << systemStats.GETDISKUsed() << " GB" << std::endl;
                std::cout << "Disk Utilization: " << systemStats.GETDISKUtilization() << " %" << std::endl;
                break;

            case 4:  // NETWORK
                std::cout << "\n--- Testing NETWORK ---\n";
                std::cout << "Network Send Speed: " << systemStats.GETNETWORKSend() << std::endl;
                std::cout << "Network Receive Speed: " << systemStats.GETNETWORKReceive() << std::endl;
                break;

            case 0:  // Exit
                std::cout << "Exiting the program." << std::endl;
                break;

            default:
                // Shouldn't happen due to input validation, but kept for safety
                std::cout << "Invalid choice. Please choose a valid option (1-4)." << std::endl;
                break;
        }

    } while (choice != 0); // Keep running until the user chooses to exit (option 0)

    return 0;
}
