#include "raylib.h"
#include "BarV1.h"
#include "GaugeV1.h"
#include "Stats.h"
#include <string>
#include <cmath>
#include <algorithm>

int main() {
    // Window initialization
    const int baseWidth = 800;
    const int baseHeight = 600;
    int screenWidth = baseWidth;
    int screenHeight = baseHeight;

    InitWindow(screenWidth, screenHeight, "Performance Monitor - GraphicV2");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(300, 300);
    SetTargetFPS(30);  

    // Stats instance
    stats stats;

    // Gauge instances
    GaugeV1::Theme Gauge_Theme;
    GaugeV1::Dimensions Gauge_Dimesions;
    GaugeV1 Gauge_CPU_Util(Gauge_Theme, Gauge_Dimesions, GaugeV1::Config::ConfigArc());
    GaugeV1 Gauge_RAM_Util(Gauge_Theme, Gauge_Dimesions, GaugeV1::Config::ConfigQuarter());

    // Bar instances
    BarV1::Theme Bar_Themes;
    BarV1::Dimensions Bar_Dimesions;
    BarV1::Config Bar_config;
    BarV1 Bar_CPU_Freq(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Bar_Wifi_Send(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Bar_Wifi_Recv(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Bar_Ether_Send(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Bar_Ether_Recv(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Bar_Disk_Util(Bar_Themes, Bar_Dimesions, Bar_config);

    // Values to display
    float CPU_Freq = 0.0;
    float CPU_Util = 0.0;
    float RAM_Util = 0.0;
    float Wifi_Send = 0.0;
    float Wifi_Recv = 0.0;
    float Ether_Send = 0.0;
    float Ether_Recv = 0.0;
    float Disk_Util = 0.0;

    // Timer to track update interval
    float updateTimer = 0.0f;
    const float updateInterval = 5.0f; // Update every 5 seconds

    while (!WindowShouldClose()) {
        // Update values every 5 seconds
        if (GetTime() - updateTimer >= updateInterval) {
            // Update stats values only every 5 seconds
            CPU_Freq = 0.0;
            CPU_Util = 0.0;
            RAM_Util = 0.0;
            Wifi_Send = 0.0;
            Wifi_Recv = 0.0;
            Ether_Send = 0.0;
            Ether_Recv = 0.0;
            Disk_Util = 0.0;

            // Update the timer
            updateTimer = GetTime();
        }

        // Update gauge and bar values
        Gauge_CPU_Util.setValue(CPU_Util);
        Gauge_RAM_Util.setValue(RAM_Util);
        Bar_CPU_Freq.setValue(CPU_Freq);
        Bar_Wifi_Send.setValue(Wifi_Send);
        Bar_Wifi_Recv.setValue(Wifi_Recv);
        Bar_Ether_Send.setValue(Ether_Send);
        Bar_Ether_Recv.setValue(Ether_Recv);
        Bar_Disk_Util.setValue(Disk_Util);

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);

        // Get screen dimensions only if necessary (e.g., resizing window)
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        float heightScale = screenHeight / baseHeight;

        // Center coordinates for quadrants
        Vector2 Pos_centreQuad1 = { screenWidth * 0.25f, screenHeight * 0.25f };
        Vector2 Pos_centreQuad2 = { screenWidth * 0.75f, screenHeight * 0.25f };
        Vector2 Pos_centreQuad3 = { screenWidth * 0.25f, screenHeight * 0.75f };
        Vector2 Pos_centreQuad4 = { screenWidth * 0.75f, screenHeight * 0.75f };
        float verticalOffset = screenHeight / 5;

        // Font size based on screen height
        int fontSize = (int)(20 * heightScale);
        fontSize = std::max(fontSize, 10);
        int titlefontsize = fontSize * 1.2;

        // Draw quadrant lines
        DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE);
        DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

        // Quad 1 - CPU
        const char* CPU_text_title = "CPU";
        int size_CPU_text_title = MeasureText(CPU_text_title, titlefontsize);
        DrawText(CPU_text_title, Pos_centreQuad1.x - size_CPU_text_title / 2, Pos_centreQuad1.y - verticalOffset - titlefontsize / 2, titlefontsize, WHITE);
        Gauge_CPU_Util.draw(Pos_centreQuad1, "Utilization");
        Bar_CPU_Freq.draw({ Pos_centreQuad1.x, Pos_centreQuad1.y + verticalOffset * 0.8f }, "Frequency", std::to_string(static_cast<int>(CPU_Freq)));

        // Quad 2 - RAM
        const char* RAM_text_title = "RAM";
        int size_RAM_text_title = MeasureText(RAM_text_title, titlefontsize);
        DrawText(RAM_text_title, Pos_centreQuad2.x - size_RAM_text_title / 2, Pos_centreQuad2.y - verticalOffset - titlefontsize / 2, titlefontsize, WHITE);
        Gauge_RAM_Util.draw(Pos_centreQuad2, "Load");

        // Quad 3 - Network
        const char* Network_text_title = "Network";
        int size_Network_text_title = MeasureText(Network_text_title, fontSize);
        DrawText(Network_text_title, Pos_centreQuad3.x - size_Network_text_title / 2, Pos_centreQuad3.y - verticalOffset - titlefontsize / 2, titlefontsize, WHITE);
        const char* WifiSendLabel = "WiFi Send";
        Bar_Wifi_Send.draw({ Pos_centreQuad3.x, Pos_centreQuad3.y - 0.53f * (verticalOffset) }, WifiSendLabel, std::to_string(static_cast<int>(Wifi_Send)));
        const char* WifiRecvLabel = "WiFi Recv";
        Bar_Wifi_Recv.draw({ Pos_centreQuad3.x, Pos_centreQuad3.y - 0.12f * (verticalOffset) }, WifiRecvLabel, std::to_string(static_cast<int>(Wifi_Recv)));
        const char* EtherSendLabel = "Eth Send";
        Bar_Ether_Send.draw({ Pos_centreQuad3.x, Pos_centreQuad3.y + 0.30f * (verticalOffset) }, EtherSendLabel, std::to_string(static_cast<int>(Ether_Send)));
        const char* EtherRecvLabel = "Eth Recv";
        Bar_Ether_Recv.draw({ Pos_centreQuad3.x, Pos_centreQuad3.y + 0.72f * (verticalOffset) }, EtherRecvLabel, std::to_string(static_cast<int>(Ether_Recv)));

        // Quad 4 - Disk
        const char* Disk_text_title = "Disk";
        int size_Disk_text_title = MeasureText(Disk_text_title, fontSize);
        DrawText(Disk_text_title, Pos_centreQuad4.x - size_Disk_text_title / 2, Pos_centreQuad4.y - verticalOffset - titlefontsize / 2, titlefontsize, WHITE);
        const char* DiskUtilLabel = "C: Utilization";
        Bar_Disk_Util.draw({ Pos_centreQuad4.x, Pos_centreQuad4.y }, DiskUtilLabel, std::to_string(static_cast<int>(Disk_Util)));

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
