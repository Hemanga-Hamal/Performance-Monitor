#include "raylib.h"
#include "Gauge.h"
#include "Bar.h"

void updateMetrics(float& cpu, float& ram, float& network, float& disk, int frameCounter) {
    cpu     = (frameCounter % 101);
    ram     = (frameCounter % 101);
    network = (frameCounter % 101);
    disk    = (frameCounter % 101);
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

    Gauge::Config configCPU, configRAM, configNetwork, configDisk;
    configCPU.autoScale = true;
    configRAM.autoScale = true;
    configNetwork.autoScale = true;
    configDisk.autoScale = true;

    Gauge gaugeCPU(Gauge::Theme(), dims, configCPU);
    Gauge gaugeRAM(Gauge::Theme(), dims, configRAM);
    Gauge gaugeNetwork(Gauge::Theme(), dims, configNetwork);
    Gauge gaugeDisk(Gauge::Theme(), dims, configDisk);

    // Bar dimensions and config
    Bar::Theme barTheme;
    Bar::Dimensions barDimensions;
    const float BASE_BAR_WIDTH = 150;
    const float BASE_BAR_HEIGHT = 20;
    barDimensions.barWidth = BASE_BAR_WIDTH;
    barDimensions.barHeight = BASE_BAR_HEIGHT;

    Bar cpuBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar ramBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar networkBar(barTheme, barDimensions, Bar::Config(0, 100));
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
        networkBar.setDimensions(barDimensions);
        diskBar.setDimensions(barDimensions);

        // Calculate quadrant positions
        Vector2 quad1 = { (float)(screenWidth / 4), (float)(screenHeight / 4) };
        Vector2 quad2 = { (float)((screenWidth / 4) * 3), (float)(screenHeight / 4) };
        Vector2 quad3 = { (float)(screenWidth / 4), (float)((screenHeight / 4) * 3) };
        Vector2 quad4 = { (float)((screenWidth / 4) * 3), (float)((screenHeight / 4) * 3) };

        // Update metrics dynamically
        float cpu, ram, network, disk;
        updateMetrics(cpu, ram, network, disk, frameCounter);

        // Set bar values dynamically
        cpuBar.setConfig(Bar::Config(cpu, 100));
        ramBar.setConfig(Bar::Config(ram, 100));
        networkBar.setConfig(Bar::Config(network, 100));
        diskBar.setConfig(Bar::Config(disk, 100));

        // Set gauge values
        gaugeCPU.setValue(cpu);
        gaugeRAM.setValue(ram);
        gaugeNetwork.setValue(network);
        gaugeDisk.setValue(disk);

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
            Vector2 networkPosition = { quad3.x - (barDimensions.barWidth / 2), quad3.y + verticalOffset * 0.0f };
            Vector2 diskPosition = { quad4.x - (barDimensions.barWidth / 2), quad4.y + verticalOffset * 0.0f };

            // Calculate bar centers
            Vector2 cpuBarCenter = { cpuPosition.x + barDimensions.barWidth/2, cpuPosition.y + barDimensions.barHeight/2 };
            Vector2 ramBarCenter = { ramPosition.x + barDimensions.barWidth/2, ramPosition.y + barDimensions.barHeight/2 };
            Vector2 networkBarCenter = { networkPosition.x + barDimensions.barWidth/2, networkPosition.y + barDimensions.barHeight/2 };
            Vector2 diskBarCenter = { diskPosition.x + barDimensions.barWidth/2, diskPosition.y + barDimensions.barHeight/2 };

            // Draw bars
            cpuBar.draw(cpuPosition);
            ramBar.draw(ramPosition);
            networkBar.draw(networkPosition);
            diskBar.draw(diskPosition);

            // Draw gauges in each quadrant
            gaugeCPU.draw(quad1);
            gaugeRAM.draw(quad2);
            /* 
            gaugeNetwork.draw(quad3);
            gaugeDisk.draw(quad4);
            */

            // Draw text labels
            const char* CPU = "CPU";
            const char* RAM = "RAM";
            const char* Network = "Network";
            const char* Disk = "Disk";

            int fontSize = (int)(20 * heightScale);
            fontSize = (fontSize < 10) ? 10 : fontSize;

            int textWidth1 = MeasureText(CPU, fontSize);
            int textWidth2 = MeasureText(RAM, fontSize);
            int textWidth3 = MeasureText(Network, fontSize);
            int textWidth4 = MeasureText(Disk, fontSize);

            float labelOffset = verticalOffset * 2;

            // Calculate text positions
            Vector2 cpuTextPos = { quad1.x - textWidth1 / 2, cpuPosition.y - labelOffset };
            Vector2 ramTextPos = { quad2.x - textWidth2 / 2, ramPosition.y - labelOffset };
            Vector2 networkTextPos = { quad3.x - textWidth3 / 2, networkPosition.y - labelOffset + verticalOffset * 0.80f };
            Vector2 diskTextPos = { quad4.x - textWidth4 / 2, diskPosition.y - labelOffset + verticalOffset * 0.80f };

            // Calculate text centers
            Vector2 cpuTextCenter = { cpuTextPos.x + textWidth1/2, cpuTextPos.y + fontSize/2 };
            Vector2 ramTextCenter = { ramTextPos.x + textWidth2/2, ramTextPos.y + fontSize/2 };
            Vector2 networkTextCenter = { networkTextPos.x + textWidth3/2, networkTextPos.y + fontSize/2 };
            Vector2 diskTextCenter = { diskTextPos.x + textWidth4/2, diskTextPos.y + fontSize/2 };

            // Draw texts
            DrawText(CPU, cpuTextPos.x, cpuTextPos.y, fontSize, WHITE);
            DrawText(RAM, ramTextPos.x, ramTextPos.y, fontSize, WHITE);
            DrawText(Network, networkTextPos.x, networkTextPos.y, fontSize, WHITE);
            DrawText(Disk, diskTextPos.x, diskTextPos.y, fontSize, WHITE);

            // Draw center dots
            // Gauge centers (Red)
            DrawCircle(quad1.x, quad1.y, 3, RED);
            DrawCircle(quad2.x, quad2.y, 3, RED);
            /*
            DrawCircle(quad3.x, quad3.y, 3, RED);
            DrawCircle(quad4.x, quad4.y, 3, RED);
            */

            // Bar centers (Green)
            DrawCircle(cpuBarCenter.x, cpuBarCenter.y, 3, GREEN);
            DrawCircle(ramBarCenter.x, ramBarCenter.y, 3, GREEN);
            DrawCircle(networkBarCenter.x, networkBarCenter.y, 3, GREEN);
            DrawCircle(diskBarCenter.x, diskBarCenter.y, 3, GREEN);

            // Text centers (Blue)
            DrawCircle(cpuTextCenter.x, cpuTextCenter.y, 3, BLUE);
            DrawCircle(ramTextCenter.x, ramTextCenter.y, 3, BLUE);
            DrawCircle(networkTextCenter.x, networkTextCenter.y, 3, BLUE);
            DrawCircle(diskTextCenter.x, diskTextCenter.y, 3, BLUE);

        EndDrawing();

        frameCounter++;
    }

    CloseWindow();
    return 0;
}