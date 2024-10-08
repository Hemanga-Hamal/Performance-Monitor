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

        barDimensions.barWidth = BASE_BAR_WIDTH * widthScale;
        barDimensions.barHeight = BASE_BAR_HEIGHT * heightScale;

        cpuBar.setDimensions(barDimensions);
        ramBar.setDimensions(barDimensions);
        networkBar.setDimensions(barDimensions);
        diskBar.setDimensions(barDimensions);

        Vector2 quad1 = { (float)(screenWidth / 4), (float)(screenHeight / 4) };
        Vector2 quad2 = { (float)((screenWidth / 4) * 3), (float)(screenHeight / 4) };
        Vector2 quad3 = { (float)(screenWidth / 4), (float)((screenHeight / 4) * 3) };
        Vector2 quad4 = { (float)((screenWidth / 4) * 3), (float)((screenHeight / 4) * 3) };

        float cpu, ram, network, disk;
        updateMetrics(cpu, ram, network, disk, frameCounter);

        cpuBar.setConfig(Bar::Config(cpu, 100));
        ramBar.setConfig(Bar::Config(ram, 100));
        networkBar.setConfig(Bar::Config(network, 100));
        diskBar.setConfig(Bar::Config(disk, 100));

        BeginDrawing();
            ClearBackground(BLACK);

            DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE);
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

            float verticalOffset = screenHeight / 8;
            
            Vector2 cpuPosition = { 
                quad1.x - (barDimensions.barWidth / 2), 
                quad1.y - (barDimensions.barHeight / 2) + verticalOffset * heightScale
            };
            Vector2 ramPosition = { 
                quad2.x - (barDimensions.barWidth / 2), 
                quad2.y - (barDimensions.barHeight / 2) + verticalOffset * heightScale
            };
            Vector2 networkPosition = { 
                quad3.x - (barDimensions.barWidth / 2), 
                quad3.y - (barDimensions.barHeight / 2) + verticalOffset * heightScale
            };
            Vector2 diskPosition = { 
                quad4.x - (barDimensions.barWidth / 2), 
                quad4.y - (barDimensions.barHeight / 2) + verticalOffset * heightScale
            };

            cpuBar.draw(cpuPosition);
            ramBar.draw(ramPosition);
            networkBar.draw(networkPosition);
            diskBar.draw(diskPosition);

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

            float labelOffset = verticalOffset * heightScale * 2;
            
            DrawText(CPU, quad1.x - textWidth1 / 2, 
                    cpuPosition.y - labelOffset, fontSize, WHITE);
            DrawText(RAM, quad2.x - textWidth2 / 2, 
                    ramPosition.y - labelOffset, fontSize, WHITE);
            DrawText(Network, quad3.x - textWidth3 / 2, 
                    networkPosition.y - labelOffset, fontSize, WHITE);
            DrawText(Disk, quad4.x - textWidth4 / 2, 
                    diskPosition.y - labelOffset, fontSize, WHITE);

        EndDrawing();

        frameCounter++;
    }

    CloseWindow();
    return 0;
}