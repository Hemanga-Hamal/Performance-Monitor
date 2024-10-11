#include "raylib.h"
#include "Gauge.h"
#include "Bar.h"

void updateMetrics(float& cpu, float& ram, float& WifiSend, float& WifiRecv, 
                  float& EtherSend, float& EtherRecv, float& disk, int frameCounter) {
    cpu = (frameCounter % 101);
    ram = (frameCounter % 101);
    WifiSend = (frameCounter % 101);
    WifiRecv = (frameCounter % 101);
    EtherSend = (frameCounter % 101);
    EtherRecv = (frameCounter % 101);
    disk = (frameCounter % 101);
}

int main() {
    // Window initialization
    int baseWidth = 800;
    int baseHeight = 600;
    int screenWidth = baseWidth;
    int screenHeight = baseHeight;

    InitWindow(screenWidth, screenHeight, "Performance Monitor - Combined Layout");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(200, 200);
    SetTargetFPS(60);

    // Gauge setup
    Gauge::Dimensions dims;
    dims.baseSize = 200.0f;
    dims.scaleRatio = 1.0f;
    dims.arcThickness = 0.15f;
    dims.textSizeRatio = 0.25f;

    Gauge::Config configCPU, configRAM;
    configCPU.autoScale = true;
    configRAM.autoScale = true;

    Gauge gaugeCPU(Gauge::Theme(), dims, configCPU);
    Gauge gaugeRAM(Gauge::Theme(), dims, configRAM);

    // Bar setup
    const float BASE_BAR_WIDTH = 150;
    const float BASE_BAR_HEIGHT = 20;
    
    Bar::Theme barTheme;
    Bar::Dimensions barDimensions;
    barDimensions.barWidth = BASE_BAR_WIDTH;
    barDimensions.barHeight = BASE_BAR_HEIGHT;
    
    Bar cpuBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar ramBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar WifiSendBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar WifiRecvBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar EtherSendBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar EtherRecvBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar diskBar(barTheme, barDimensions, Bar::Config(0, 100));

    // Label definitions
    const char* CPU = "CPU";
    const char* RAM = "RAM";
    const char* Network = "Network";
    const char* WifiSendLabel = "WiFi Send";
    const char* WifiRecvLabel = "WiFi Recv";
    const char* EtherSendLabel = "Eth Send";
    const char* EtherRecvLabel = "Eth Recv";
    const char* Disk = "Disk";

    int frameCounter = 0;

    while (!WindowShouldClose()) {
        // Update window dimensions
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        float widthScale = (float)screenWidth / baseWidth;
        float heightScale = (float)screenHeight / baseHeight;

        // Update bar dimensions
        barDimensions.barWidth = BASE_BAR_WIDTH * widthScale;
        barDimensions.barHeight = BASE_BAR_HEIGHT * heightScale;

        cpuBar.setDimensions(barDimensions);
        ramBar.setDimensions(barDimensions);
        WifiSendBar.setDimensions(barDimensions);
        WifiRecvBar.setDimensions(barDimensions);
        EtherSendBar.setDimensions(barDimensions);
        EtherRecvBar.setDimensions(barDimensions);
        diskBar.setDimensions(barDimensions);

        // Calculate quadrant positions
        Vector2 quad1 = { (float)(screenWidth / 4), (float)(screenHeight / 4) };
        Vector2 quad2 = { (float)((screenWidth / 4) * 3), (float)(screenHeight / 4) };
        Vector2 quad3 = { (float)(screenWidth / 4), (float)((screenHeight / 4) * 3) };
        Vector2 quad4 = { (float)((screenWidth / 4) * 3), (float)((screenHeight / 4) * 3) };

        // Update metrics
        float cpu, ram, WifiSend, WifiRecv, EtherSend, EtherRecv, disk;
        updateMetrics(cpu, ram, WifiSend, WifiRecv, EtherSend, EtherRecv, disk, frameCounter);

        // Update component values
        gaugeCPU.setValue(cpu);
        gaugeRAM.setValue(ram);
        
        cpuBar.setConfig(Bar::Config(cpu, 100));
        ramBar.setConfig(Bar::Config(ram, 100));
        WifiSendBar.setConfig(Bar::Config(WifiSend, 100));
        WifiRecvBar.setConfig(Bar::Config(WifiRecv, 100));
        EtherSendBar.setConfig(Bar::Config(EtherSend, 100));
        EtherRecvBar.setConfig(Bar::Config(EtherRecv, 100));
        diskBar.setConfig(Bar::Config(disk, 100));

        BeginDrawing();
        {
            ClearBackground(BLACK);

            // Draw quadrant lines
            DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE);
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

            // Calculate font size
            int fontSize = (int)(20 * heightScale);
            fontSize = (fontSize < 10) ? 10 : fontSize;

            // Calculate common offsets
            float verticalOffset = screenHeight / 5;
            float networkBaseOffset = verticalOffset * 0.4f;
            float networkSpacing = verticalOffset * 0.4f;
            float labelOffset = fontSize * 1.5f;

            // Measure text widths
            int textWidth1 = MeasureText(CPU, fontSize);
            int textWidth2 = MeasureText(RAM, fontSize);
            int textWidth3 = MeasureText(Network, fontSize);
            int textWidthWifiSend = MeasureText(WifiSendLabel, fontSize);
            int textWidthWifiRecv = MeasureText(WifiRecvLabel, fontSize);
            int textWidthEtherSend = MeasureText(EtherSendLabel, fontSize);
            int textWidthEtherRecv = MeasureText(EtherRecvLabel, fontSize);
            int textWidth4 = MeasureText(Disk, fontSize);

            // Draw gauges
            gaugeCPU.draw(quad1);
            gaugeRAM.draw(quad2);

            // Calculate positions
            Vector2 cpuPosition = { quad1.x - (barDimensions.barWidth / 2), quad1.y + verticalOffset * 0.80f };
            Vector2 ramPosition = { quad2.x - (barDimensions.barWidth / 2), quad2.y + verticalOffset * 0.80f };
            Vector2 networkPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + verticalOffset * 0.80f };
            Vector2 wifiSendPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + networkBaseOffset - screenHeight/5 };
            Vector2 wifiRecvPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + networkBaseOffset + networkSpacing * 1 - screenHeight/5 };
            Vector2 etherSendPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + networkBaseOffset + networkSpacing * 2 - screenHeight/5 };
            Vector2 etherRecvPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + networkBaseOffset + networkSpacing * 3 - screenHeight/5 };
            Vector2 diskPosition = { quad4.x - (barDimensions.barWidth / 2), quad4.y + verticalOffset * 0.80f };

            // Calculate text positions
            Vector2 cpuTextPos = { quad1.x - textWidth1 / 2, cpuPosition.y - labelOffset - screenHeight/3 };
            Vector2 ramTextPos = { quad2.x - textWidth2 / 2, ramPosition.y - labelOffset - screenHeight/3 };
            Vector2 networkTextPos = { quad3.x - textWidth3 / 2, networkPosition.y - labelOffset - screenHeight/3 };
            Vector2 wifiSendTextPos = { quad3.x - textWidthWifiSend * 0.8f - barDimensions.barWidth * 0.7f, wifiSendPosition.y };
            Vector2 wifiRecvTextPos = { quad3.x - textWidthWifiRecv * 0.8f - barDimensions.barWidth * 0.7f, wifiRecvPosition.y };
            Vector2 etherSendTextPos = { quad3.x - textWidthEtherSend * 0.8f - barDimensions.barWidth * 0.7f, etherSendPosition.y };
            Vector2 etherRecvTextPos = { quad3.x - textWidthEtherRecv * 0.8f - barDimensions.barWidth * 0.7f, etherRecvPosition.y };
            Vector2 diskTextPos = { quad4.x - textWidth4 / 2, diskPosition.y - labelOffset - screenHeight/3 };

            // Calculate network centering
            float NETWORKCentering = ((quad3.x - wifiSendTextPos.x)/2) - 
                                   (quad3.x - (wifiSendTextPos.x + wifiSendPosition.x + barDimensions.barWidth)/2);

            // Draw bars
            cpuBar.draw(cpuPosition);
            ramBar.draw(ramPosition);
            WifiSendBar.draw({wifiSendPosition.x + NETWORKCentering, wifiSendPosition.y});
            WifiRecvBar.draw({wifiRecvPosition.x + NETWORKCentering, wifiRecvPosition.y});
            EtherSendBar.draw({etherSendPosition.x + NETWORKCentering, etherSendPosition.y});
            EtherRecvBar.draw({etherRecvPosition.x + NETWORKCentering, etherRecvPosition.y});
            diskBar.draw({diskPosition.x, quad4.y});

            // Draw texts
            DrawText(CPU, cpuTextPos.x, cpuTextPos.y, fontSize, WHITE);
            DrawText(RAM, ramTextPos.x, ramTextPos.y, fontSize, WHITE);
            DrawText(Network, networkTextPos.x, networkTextPos.y, fontSize, WHITE);
            DrawText(WifiSendLabel, wifiSendTextPos.x + NETWORKCentering, wifiSendTextPos.y, fontSize, WHITE);
            DrawText(WifiRecvLabel, wifiRecvTextPos.x + NETWORKCentering, wifiRecvTextPos.y, fontSize, WHITE);
            DrawText(EtherSendLabel, etherSendTextPos.x + NETWORKCentering, etherSendTextPos.y, fontSize, WHITE);
            DrawText(EtherRecvLabel, etherRecvTextPos.x + NETWORKCentering, etherRecvTextPos.y, fontSize, WHITE);
            DrawText(Disk, diskTextPos.x, diskTextPos.y, fontSize, WHITE);

        EndDrawing();

        frameCounter++;
    }

    CloseWindow();
    return 0;
}