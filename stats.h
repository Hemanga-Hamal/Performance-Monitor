#ifndef STATS_H
#define STATS_H

#include <iostream>
#include <windows.h>
#include <intrin.h>
#include <conio.h>

class stats{
protected:
    //CPU
    double CPUFrequency;
    double CPUUtilization;
    
    //RAM
    double RAMTotal;
    double RAMUsed;
    double RAMUtilization;



public:
    stats();
    ~stats();

    //CPU
    double GetCPUFrequency();
    double GetCPUUtilization();

    //RAM
    double GETRAMTotal();
    double GETRAMUsed();
    double GETRAMUtilization();


};

#endif