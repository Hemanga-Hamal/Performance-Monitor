#include "raylib.h"
#include "Gauge.h"
#include "Bar.h"

void updateMetrics(float& cpu, float& ram, float& WifiSend, float& WifiRecv, float& EtherSend, float& EtherRecv, float& disk, int frameCounter) {
    cpu = (frameCounter % 101);
    ram = (frameCounter % 101);
    WifiSend = (frameCounter % 101);
    WifiRecv = (frameCounter % 101);
    EtherSend = (frameCounter % 101);
    EtherRecv = (frameCounter % 101);
    disk = (frameCounter % 101);
}

int main() {
    int baseWidth = 800;
    int baseHeight = 600;

    int screenWidth = baseWidth;
    int screenHeight = baseHeight;

    InitWindow(screenWidth, screenHeight, "Performance Monitor - Combined Layout");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(200, 200);
    SetTargetFPS(60);

    // Gauge dimensions and config
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

    // Bar dimensions and config
    Bar::Theme barTheme;
    Bar::Dimensions barDimensions;
    const float BASE_BAR_WIDTH = 150;
    const float BASE_BAR_HEIGHT = 20;
    barDimensions.barWidth = BASE_BAR_WIDTH;
    barDimensions.barHeight = BASE_BAR_HEIGHT;

    Bar cpuBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar ramBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar WifiSendBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar WifiRecvBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar EtherSendBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar EtherRecvBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar diskBar(barTheme, barDimensions, Bar::Config(0, 100));

    int frameCounter = 0;

    while (!WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        float xScale = (float)screenWidth / baseWidth;
        float yScale = (float)screenHeight / baseHeight;
        float widthScale = xScale;
        float heightScale = yScale;

        // Adjust bar dimensions for dynamic scaling
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

        // Update metrics dynamically
        float cpu, ram, WifiSend, WifiRecv, EtherSend, EtherRecv, disk;
        updateMetrics(cpu, ram, WifiSend, WifiRecv, EtherSend, EtherRecv, disk, frameCounter);

        // Set bar values dynamically
        cpuBar.setConfig(Bar::Config(cpu, 100));
        ramBar.setConfig(Bar::Config(ram, 100));
        WifiSendBar.setConfig(Bar::Config(WifiSend, 100));
        WifiRecvBar.setConfig(Bar::Config(WifiRecv, 100));
        EtherSendBar.setConfig(Bar::Config(EtherSend, 100));
        EtherRecvBar.setConfig(Bar::Config(EtherRecv, 100));
        diskBar.setConfig(Bar::Config(disk, 100));

        // Set gauge values
        gaugeCPU.setValue(cpu);
        gaugeRAM.setValue(ram);

        BeginDrawing();
        ClearBackground(BLACK);

        // Draw quadrant lines
        DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE);
        DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

        // Vertical offset for bar placement
        float verticalOffset = screenHeight / 5;

        // Bar positions
        Vector2 cpuPosition = { quad1.x - (barDimensions.barWidth / 2), quad1.y + verticalOffset * 0.80f };
        Vector2 ramPosition = { quad2.x - (barDimensions.barWidth / 2), quad2.y + verticalOffset * 0.80f };

        // Network bars with even spacing
        float networkBaseOffset = verticalOffset * 0.4f;
        float networkSpacing = verticalOffset * 0.4f;

        Vector2 networkPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + verticalOffset * 0.80f };
        Vector2 wifiSendPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + networkBaseOffset - screenHeight/5 };
        Vector2 wifiRecvPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + networkBaseOffset + networkSpacing * 1 - screenHeight/5 };
        Vector2 etherSendPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + networkBaseOffset + networkSpacing * 2  - screenHeight/5};
        Vector2 etherRecvPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + networkBaseOffset + networkSpacing * 3  - screenHeight/5};

        Vector2 diskPosition = { quad4.x - (barDimensions.barWidth / 2), quad4.y + verticalOffset * 0.80f };

        // Calculate bar centers
        Vector2 cpuBarCenter = { cpuPosition.x + barDimensions.barWidth/2, cpuPosition.y + barDimensions.barHeight/2 };
        Vector2 ramBarCenter = { ramPosition.x + barDimensions.barWidth/2, ramPosition.y + barDimensions.barHeight/2 };
        Vector2 WifiSendBarCenter = { wifiSendPosition.x + barDimensions.barWidth/2, wifiSendPosition.y + barDimensions.barHeight/2 };
        Vector2 WifiRecvBarCenter = { wifiRecvPosition.x + barDimensions.barWidth/2, wifiRecvPosition.y + barDimensions.barHeight/2 };
        Vector2 EtherSendBarCenter = { etherSendPosition.x + barDimensions.barWidth/2, etherSendPosition.y + barDimensions.barHeight/2 };
        Vector2 EtherRecvBarCenter = { etherRecvPosition.x + barDimensions.barWidth/2, etherRecvPosition.y + barDimensions.barHeight/2 };
        Vector2 diskBarCenter = { diskPosition.x + barDimensions.barWidth/2, diskPosition.y + barDimensions.barHeight/2 };

        // Draw bars
        cpuBar.draw(cpuPosition);
        ramBar.draw(ramPosition);
        WifiSendBar.draw(wifiSendPosition);
        WifiRecvBar.draw(wifiRecvPosition);
        EtherSendBar.draw(etherSendPosition);
        EtherRecvBar.draw(etherRecvPosition);
        diskBar.draw(diskPosition);

        // Draw gauges in each quadrant
        gaugeCPU.draw(quad1);
        gaugeRAM.draw(quad2);

        // Draw text labels
        const char* CPU = "CPU";
        const char* RAM = "RAM";
        const char* Network = "Network";
        const char* WifiSendLabel = "WiFi Send";
        const char* WifiRecvLabel = "WiFi Recv";
        const char* EtherSendLabel = "Eth Send";
        const char* EtherRecvLabel = "Eth Recv";
        const char* Disk = "Disk";

        int fontSize = (int)(20 * heightScale);
        fontSize = (fontSize < 10) ? 10 : fontSize;

        int textWidth1 = MeasureText(CPU, fontSize);
        int textWidth2 = MeasureText(RAM, fontSize);
        int textWidth3 = MeasureText(Network, fontSize);
        int textWidthWifiSend = MeasureText(WifiSendLabel, fontSize);
        int textWidthWifiRecv = MeasureText(WifiRecvLabel, fontSize);
        int textWidthEtherSend = MeasureText(EtherSendLabel, fontSize);
        int textWidthEtherRecv = MeasureText(EtherRecvLabel, fontSize);
        int textWidth4 = MeasureText(Disk, fontSize);

        float labelOffset = fontSize * 1.5f;  // Adjusted for better text positioning

        // Calculate text positions
        Vector2 cpuTextPos = { quad1.x - textWidth1 / 2, cpuPosition.y - labelOffset - screenHeight/3 };
        Vector2 ramTextPos = { quad2.x - textWidth2 / 2, ramPosition.y - labelOffset - screenHeight/3 };
        Vector2 networkTextPos = { quad3.x - textWidth3 / 2, networkPosition.y - labelOffset - screenHeight/3 };
        Vector2 wifiSendTextPos = { quad3.x - textWidthWifiSend * 0.8f - barDimensions.barWidth * 0.7f, wifiSendPosition.y };
        Vector2 wifiRecvTextPos = { quad3.x - textWidthWifiRecv * 0.8f - barDimensions.barWidth * 0.7f, wifiRecvPosition.y };
        Vector2 etherSendTextPos = { quad3.x - textWidthEtherSend * 0.8f - barDimensions.barWidth * 0.7f, etherSendPosition.y };
        Vector2 etherRecvTextPos = { quad3.x - textWidthEtherRecv * 0.8f - barDimensions.barWidth * 0.7f, etherRecvPosition.y };
        Vector2 diskTextPos = { quad4.x - textWidth4 / 2, diskPosition.y - labelOffset - screenHeight/3 };

        // Calculate text centers
        Vector2 cpuTextCenter = { cpuTextPos.x + textWidth1/2, cpuTextPos.y + fontSize/2 };
        Vector2 ramTextCenter = { ramTextPos.x + textWidth2/2, ramTextPos.y + fontSize/2 };
        Vector2 networkTextCenter = { networkTextPos.x + textWidth3/2, networkTextPos.y + fontSize/2 };
        Vector2 diskTextCenter = { diskTextPos.x + textWidth4/2, diskTextPos.y + fontSize/2 };

        // Draw texts
        DrawText(CPU, cpuTextPos.x, cpuTextPos.y, fontSize, WHITE);
        DrawText(RAM, ramTextPos.x, ramTextPos.y, fontSize, WHITE);
        DrawText(Network, networkTextPos.x, networkTextPos.y, fontSize, WHITE);
        DrawText(WifiSendLabel, wifiSendTextPos.x, wifiSendTextPos.y, fontSize, WHITE);
        DrawText(WifiRecvLabel, wifiRecvTextPos.x, wifiRecvTextPos.y, fontSize, WHITE);
        DrawText(EtherSendLabel, etherSendTextPos.x, etherSendTextPos.y, fontSize, WHITE);
        DrawText(EtherRecvLabel, etherRecvTextPos.x, etherRecvTextPos.y, fontSize, WHITE);
        DrawText(Disk, diskTextPos.x, diskTextPos.y, fontSize, WHITE);

        // Draw center dots
        // Gauge centers (Red)
        DrawCircle(quad1.x, quad1.y, 3, RED);
        DrawCircle(quad2.x, quad2.y, 3, RED);

        // Bar centers (Green)
        DrawCircle(cpuBarCenter.x, cpuBarCenter.y, 3, GREEN);
        DrawCircle(ramBarCenter.x, ramBarCenter.y, 3, GREEN);

        DrawCircle(WifiSendBarCenter.x, WifiSendBarCenter.y, 3, GREEN);
        DrawCircle(WifiRecvBarCenter.x, WifiRecvBarCenter.y, 3, GREEN);
        DrawCircle(EtherSendBarCenter.x, EtherSendBarCenter.y, 3, GREEN);
        DrawCircle(EtherRecvBarCenter.x, EtherRecvBarCenter.y, 3, GREEN);
        DrawCircle(diskBarCenter.x, diskBarCenter.y, 3, GREEN);

        // Text centers (Blue)
        DrawCircle(cpuTextCenter.x, cpuTextCenter.y, 3, BLUE);
        DrawCircle(ramTextCenter.x, ramTextCenter.y, 3, BLUE);
        DrawCircle(networkTextCenter.x, networkTextCenter.y, 3, BLUE);
        DrawCircle(wifiSendPosition.x/2 , wifiSendPosition.y + fontSize/2 , 3, BLUE);
        DrawCircle(wifiRecvPosition.x/2 , wifiRecvPosition.y + fontSize/2 , 3, BLUE);
        DrawCircle(etherSendPosition.x/2 , etherSendPosition.y + fontSize/2 , 3, BLUE);
        DrawCircle(etherRecvPosition.x/2 , etherRecvPosition.y + fontSize/2 , 3, BLUE);
        DrawCircle(diskTextCenter.x, diskTextCenter.y, 3, BLUE);

        EndDrawing();

        frameCounter++;
    }

    CloseWindow();
    return 0;
}