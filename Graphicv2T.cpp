#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include <windows.h>
#include "raylib.h"
#include "StatsV1.h"
#include "BarV1.h"
#include "GaugeV1.h"
#include <thread>
#include <atomic>
#include <string>

using RaylibVector2 = ::Vector2;
using RaylibColor = ::Color;

// Shared Data (Now Uses Atomics for Thread Safety)
struct StatsData {
    std::atomic<float> CPU_Freq{0.0f};
    std::atomic<float> CPU_Util{0.0f};
    std::atomic<float> RAM_Util{0.0f};
    std::atomic<float> Wifi_Send{0.0f};
    std::atomic<float> Wifi_Recv{0.0f};
    std::atomic<float> Ether_Send{0.0f};
    std::atomic<float> Ether_Recv{0.0f};
    std::atomic<float> Disk_Util{0.0f};
} statsData;

std::atomic<bool> running(true);
StatsV1 stats1;

// Helper Function to Convert Float to String
inline std::string formatValue(float value) {
    return std::to_string(static_cast<int>(value));
}

// Thread for Fetching Stats
void updateStats() {
    while (running) {
        statsData.CPU_Freq.store(stats1.GETCPUFrequency());
        statsData.CPU_Util.store(stats1.GETCPUtilization());
        statsData.RAM_Util.store(stats1.GETRAMUtilization());
        statsData.Wifi_Send.store(stats1.GETWiFiSend());
        statsData.Wifi_Recv.store(stats1.GETWiFiReceive());
        statsData.Ether_Send.store(stats1.GETEthernetSend());
        statsData.Ether_Recv.store(stats1.GETEthernetReceive());
        statsData.Disk_Util.store(stats1.GETDISKUtilization());

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Rendering Loop
void renderLoop() {
    const int baseWidth = 800, baseHeight = 600;
    InitWindow(baseWidth, baseHeight, "Performance Monitor - GraphicV2T");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(300, 300);
    SetTargetFPS(30);

    // Gauge Instances
    GaugeV1::Theme Gauge_Theme;
    GaugeV1::Dimensions Gauge_Dimensions;
    GaugeV1 Gauge_CPU_Util(Gauge_Theme, Gauge_Dimensions, GaugeV1::Config::ConfigArc());
    GaugeV1 Gauge_RAM_Util(Gauge_Theme, Gauge_Dimensions, GaugeV1::Config::ConfigQuarter());

    // Bar Instances
    BarV1::Theme Bar_Themes;
    BarV1::Dimensions Bar_Dimensions;
    BarV1::Config Bar_config;
    BarV1 Bar_CPU_Freq(Bar_Themes, Bar_Dimensions, Bar_config);
    BarV1 Bar_Wifi_Send(Bar_Themes, Bar_Dimensions, Bar_config);
    BarV1 Bar_Wifi_Recv(Bar_Themes, Bar_Dimensions, Bar_config);
    BarV1 Bar_Ether_Send(Bar_Themes, Bar_Dimensions, Bar_config);
    BarV1 Bar_Ether_Recv(Bar_Themes, Bar_Dimensions, Bar_config);
    BarV1 Bar_Disk_Util(Bar_Themes, Bar_Dimensions, Bar_config);

    while (!WindowShouldClose()) {
        // Cache Screen Size to Avoid Redundant Calls
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        float heightScale = screenHeight / (float)baseHeight;
        int fontSize = std::max(static_cast<int>(20 * heightScale), 10);
        int titleFontSize = static_cast<int>(fontSize * 1.2);

        // Get Latest Stats
        StatsData localStats;
        localStats.CPU_Freq = statsData.CPU_Freq.load();
        localStats.CPU_Util = statsData.CPU_Util.load();
        localStats.RAM_Util = statsData.RAM_Util.load();
        localStats.Wifi_Send = statsData.Wifi_Send.load();
        localStats.Wifi_Recv = statsData.Wifi_Recv.load();
        localStats.Ether_Send = statsData.Ether_Send.load();
        localStats.Ether_Recv = statsData.Ether_Recv.load();
        localStats.Disk_Util = statsData.Disk_Util.load();
        
        // Update UI Elements
        Gauge_CPU_Util.setValue(localStats.CPU_Util);
        Gauge_RAM_Util.setValue(localStats.RAM_Util);
        Bar_CPU_Freq.setValue(localStats.CPU_Freq);
        Bar_Wifi_Send.setValue(localStats.Wifi_Send);
        Bar_Wifi_Recv.setValue(localStats.Wifi_Recv);
        Bar_Ether_Send.setValue(localStats.Ether_Send);
        Bar_Ether_Recv.setValue(localStats.Ether_Recv);
        Bar_Disk_Util.setValue(localStats.Disk_Util);

        // Precomputed Positions
        float verticalOffset = screenHeight / 5.0f;
        RaylibVector2 centerPositions[] = {
            {screenWidth * 0.25f, screenHeight * 0.25f},
            {screenWidth * 0.75f, screenHeight * 0.25f},
            {screenWidth * 0.25f, screenHeight * 0.75f},
            {screenWidth * 0.75f, screenHeight * 0.75f}
        };

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw Grid Lines
        DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE);
        DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

        // Quad 1 - CPU
        DrawText("CPU", centerPositions[0].x - MeasureText("CPU", titleFontSize) / 2,
                 centerPositions[0].y - verticalOffset - titleFontSize / 2, titleFontSize, WHITE);
        Gauge_CPU_Util.draw(centerPositions[0], "Utilization");
        Bar_CPU_Freq.draw({ centerPositions[0].x, centerPositions[0].y + verticalOffset * 0.8f },
                          "Frequency", formatValue(localStats.CPU_Freq));

        // Quad 2 - RAM
        DrawText("RAM", centerPositions[1].x - MeasureText("RAM", titleFontSize) / 2,
                 centerPositions[1].y - verticalOffset - titleFontSize / 2, titleFontSize, WHITE);
        Gauge_RAM_Util.draw(centerPositions[1], "Load");

        // Quad 3 - Network
        DrawText("Network", centerPositions[2].x - MeasureText("Network (MBS)", titleFontSize) / 2,
                 centerPositions[2].y - verticalOffset - titleFontSize / 2, titleFontSize, WHITE);
        Bar_Wifi_Send.draw({ centerPositions[2].x, centerPositions[2].y - 0.53f * verticalOffset }, 
                           "WiFi Send", formatValue(localStats.Wifi_Send));
        Bar_Wifi_Recv.draw({ centerPositions[2].x, centerPositions[2].y - 0.12f * verticalOffset }, 
                           "WiFi Recv", formatValue(localStats.Wifi_Recv));
        Bar_Ether_Send.draw({ centerPositions[2].x, centerPositions[2].y + 0.30f * verticalOffset }, 
                            "Eth Send", formatValue(localStats.Ether_Send));
        Bar_Ether_Recv.draw({ centerPositions[2].x, centerPositions[2].y + 0.72f * verticalOffset }, 
                            "Eth Recv", formatValue(localStats.Ether_Recv));

        // Quad 4 - Disk
        DrawText("Disk", centerPositions[3].x - MeasureText("Disk", titleFontSize) / 2,
                 centerPositions[3].y - verticalOffset - titleFontSize / 2, titleFontSize, WHITE);
        Bar_Disk_Util.draw(centerPositions[3], "C: Utilization", formatValue(localStats.Disk_Util));

        EndDrawing();
    }

    running = false;
    CloseWindow();
}

int main() {
    std::thread dataThread(updateStats);
    std::thread renderThread(renderLoop);

    dataThread.join();
    renderThread.join();
    return 0;
}
