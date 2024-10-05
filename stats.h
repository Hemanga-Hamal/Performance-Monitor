#ifndef STATS_H
#define STATS_H

#include <iostream>
#include <string>

class stats {
protected:
    // CPU
    double CPUFrequency;
    double CPUUtilization;
    
    // RAM
    double RAMTotal;
    double RAMUsed;
    double RAMUtilization;

    // DISK
    double DISKTotal;
    double DISKUsed;
    double DISKUtilization;

    // NETWORK
    std::string NETWORKSourceSend;
    std::string NETWORKSourceReceive;
    double WiFiSend;
    double WiFiReceive;
    double EthernetSend;
    double EthernetReceive;

public:
    stats();
    ~stats();

    // CPU
    double GetCPUFrequency();
    double GetCPUUtilization();

    // RAM
    double GETRAMTotal();
    double GETRAMUsed();
    double GETRAMUtilization();

    // DISK
    double GETDISKTotal();
    double GETDISKUsed();
    double GETDISKUtilization();

    // NETWORK 
    double GETWiFiSend();
    double GETWiFiReceive();
    double GETEthernetSend();
    double GETEthernetReceive();
};

#endif
