#include "raylib.h"
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

    InitWindow(screenWidth, screenHeight, "Performance Monitor - Bar");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(200, 200);
    SetTargetFPS(60);

    Bar::Theme barTheme;
    Bar::Dimensions barDimensions;

    // Initialize bars without labels
    Bar cpuBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar ramBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar networkBar(barTheme, barDimensions, Bar::Config(0, 100));
    Bar diskBar(barTheme, barDimensions, Bar::Config(0, 100));

    int frameCounter = 0;

    while (!WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // Calculate scaling factors
        float xScale = (float)screenWidth / baseWidth;
        float yScale = (float)screenHeight / baseHeight;

        // Update bar dimensions dynamically based on scaling
        barDimensions.barWidth = 400 * xScale;
        barDimensions.barHeight = 30 * yScale;

        Vector2 quad1 = { (float)(screenWidth / 4), (float)(screenHeight / 4) };
        Vector2 quad2 = { (float)((screenWidth / 4) * 3), (float)(screenHeight / 4) };
        Vector2 quad3 = { (float)(screenWidth / 4), (float)((screenHeight / 4) * 3) };
        Vector2 quad4 = { (float)((screenWidth / 4) * 3), (float)((screenHeight / 4) * 3) };

        float cpu, ram, network, disk;
        updateMetrics(cpu, ram, network, disk, frameCounter);

        // Update bar values
        cpuBar.setConfig(Bar::Config(cpu, 100));
        ramBar.setConfig(Bar::Config(ram, 100));
        networkBar.setConfig(Bar::Config(network, 100));
        diskBar.setConfig(Bar::Config(disk, 100));

        BeginDrawing();
            ClearBackground(BLACK);

            // Draw grid lines
            DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE);
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

            // Calculate positions for bars in each quadrant
            Vector2 cpuPosition = { quad1.x - cpuBar.getwidth() / 2, quad1.y + screenHeight / 8 };
            Vector2 ramPosition = { quad2.x - ramBar.getwidth() / 2, quad2.y + screenHeight / 8 };
            Vector2 networkPosition = { quad3.x - networkBar.getwidth() / 2, quad3.y + screenHeight / 8 };
            Vector2 diskPosition = { quad4.x - diskBar.getwidth() / 2, quad4.y + screenHeight / 8 };

            // Draw bars
            cpuBar.draw(cpuPosition);
            ramBar.draw(ramPosition);
            networkBar.draw(networkPosition);
            diskBar.draw(diskPosition);

            // Draw labels above the bars
            const char* CPU = "CPU";
            const char* RAM = "RAM";
            const char* Network = "Network";
            const char* Disk = "Disk";

            int fontSize = (int)(20 * yScale); // Scale font size dynamically

            // Calculate text width and height for dynamic centering
            int textWidth1 = MeasureText(CPU, fontSize);
            int textWidth2 = MeasureText(RAM, fontSize);
            int textWidth3 = MeasureText(Network, fontSize);
            int textWidth4 = MeasureText(Disk, fontSize);
            int textHeight = fontSize;

            // Draw text centered in each quadrant
            DrawText(CPU, quad1.x - textWidth1 / 2, quad1.y - textHeight / 2 - screenHeight / 5, fontSize, WHITE);
            DrawText(RAM, quad2.x - textWidth2 / 2, quad2.y - textHeight / 2 - screenHeight / 5, fontSize, WHITE);
            DrawText(Network, quad3.x - textWidth3 / 2, quad3.y - textHeight / 2 - screenHeight / 5, fontSize, WHITE);
            DrawText(Disk, quad4.x - textWidth4 / 2, quad4.y - textHeight / 2 - screenHeight / 5, fontSize, WHITE);

        EndDrawing();

        frameCounter++;
    }

    CloseWindow();
    return 0;
}
