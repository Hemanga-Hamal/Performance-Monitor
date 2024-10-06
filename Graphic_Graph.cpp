#include "raylib.h"
#include "Gauge.h" 

int main() {
    // Initial screen size
    int screenWidth = 800;
    int screenHeight = 600;

    // Initialize the window
    InitWindow(screenWidth, screenHeight, "Performance Monitor - Layout with Gauges");

    // Enable window resizing and set minimum size
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(200, 200);
    SetTargetFPS(60);

    // Create custom gauges for each quadrant with different labels
    Gauge::Dimensions dims;
    dims.baseSize = 200.0f;
    dims.scaleRatio = 1.0f;   
    dims.arcThickness = 0.15f;   
    dims.textSizeRatio = 0.25f;  
    dims.labelOffset = 40.0f;

    // Custom configuration for each gauge
    Gauge::Config configCPU, configRAM, configNetwork, configDisk;
    configCPU.label = "CPU";
    configRAM.label = "RAM";
    configNetwork.label = "Network";
    configDisk.label = "Disk";

    // Create the gauges with the specific settings for each quadrant
    Gauge gaugeCPU(Gauge::Theme(), dims, configCPU);
    Gauge gaugeRAM(Gauge::Theme(), dims, configRAM);
    Gauge gaugeNetwork(Gauge::Theme(), dims, configNetwork);
    Gauge gaugeDisk(Gauge::Theme(), dims, configDisk);

    // Simulate some usage values for each gauge
    float cpuValue = 30.0f;
    float ramValue = 50.0f;    
    float networkValue = 70.0f;  
    float diskValue = 40.0f;   

    while (!WindowShouldClose()) {
        // Get current screen size (if the window is resized, these values will change)
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // Calculate the center positions for each quadrant
        Vector2 quad1 = {(float)(screenWidth / 4), (float)(screenHeight / 4)};
        Vector2 quad2 = {(float)((screenWidth / 4) * 3), (float)(screenHeight / 4)};
        Vector2 quad3 = {(float)(screenWidth / 4), (float)((screenHeight / 4) * 3)};
        Vector2 quad4 = {(float)((screenWidth / 4) * 3), (float)((screenHeight / 4) * 3)};

        // Set values for each gauge
        gaugeCPU.setValue(cpuValue);
        gaugeRAM.setValue(ramValue);
        gaugeNetwork.setValue(networkValue);
        gaugeDisk.setValue(diskValue);

        BeginDrawing();
            ClearBackground(BLACK);

            // Quadrants lines
            DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE); 
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

            // Draw the gauges in the center of each quadrant
            gaugeCPU.draw(quad1);
            gaugeRAM.draw(quad2); 
            gaugeNetwork.draw(quad3);
            gaugeDisk.draw(quad4);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
