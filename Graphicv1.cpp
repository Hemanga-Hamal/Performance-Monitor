// custom libraries
#include "raylib.h"
#include "GaugeV1.h"
// standard libraries
#include <string>
#include <cmath>
#include <algorithm>

void updateMetrics(float& test, int frameCounter) {
    test = static_cast<float>(frameCounter % 101);
}

int main() {
    // Window initialization
    const int baseWidth = 800;
    const int baseHeight = 600;
    int screenWidth = baseWidth;
    int screenHeight = baseHeight;

    InitWindow(screenWidth, screenHeight, "Performance Monitor - GaugeV1");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(300, 300);
    SetTargetFPS(60);

    int frameCounter = 0;

    // Gauge instances
    GaugeV1 gaugeArc(GaugeV1::Theme(), GaugeV1::Dimensions(), GaugeV1::Config::ConfigArc());
    GaugeV1 gaugeQuarter(GaugeV1::Theme(), GaugeV1::Dimensions(), GaugeV1::Config::ConfigQuarter());

    // Gauge value
    float testValue = 0.0f;

    while (!WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // Coordinates for each quadrant
        Vector2 quad1 = { screenWidth * 0.25f, screenHeight * 0.25f };
        Vector2 quad2 = { screenWidth * 0.75f, screenHeight * 0.25f };
        Vector2 quad3 = { screenWidth * 0.25f, screenHeight * 0.75f };
        Vector2 quad4 = { screenWidth * 0.75f, screenHeight * 0.75f };

        // Update gauge value
        updateMetrics(testValue, frameCounter);
        gaugeArc.setValue(testValue);
        gaugeQuarter.setValue(testValue);

        // Start drawing
        BeginDrawing();
        {
            ClearBackground(BLACK);

            // Draw quadrant lines
            DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE);
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

            // Draw gauges in different quadrants
            gaugeArc.draw(quad1);
            gaugeQuarter.draw(quad2); 

            // Center debug points for quadrants
            DrawCircle(static_cast<int>(quad1.x), static_cast<int>(quad1.y), 2, RED);
            DrawCircle(static_cast<int>(quad2.x), static_cast<int>(quad2.y), 2, RED);
            DrawCircle(static_cast<int>(quad3.x), static_cast<int>(quad3.y), 2, RED);
            DrawCircle(static_cast<int>(quad4.x), static_cast<int>(quad4.y), 2, RED);
        }
        EndDrawing();
        frameCounter++;
    }

    CloseWindow();
    return 0;
}
