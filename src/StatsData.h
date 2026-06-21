#ifndef STATSDATA_H
#define STATSDATA_H

#include <atomic>

struct StatsData {
    std::atomic<float> CPU_Freq{0.0f};
    std::atomic<float> CPU_Util{0.0f};
    std::atomic<float> RAM_Util{0.0f};
    std::atomic<float> Wifi_Send{0.0f};
    std::atomic<float> Wifi_Recv{0.0f};
    std::atomic<float> Ether_Send{0.0f};
    std::atomic<float> Ether_Recv{0.0f};
    std::atomic<float> GPU_Util[4]{};
    std::atomic<int> GPUCount{0};
    std::atomic<int> DiskCount{0};
    std::atomic<float> DiskUtil[8]{};
};

#endif
