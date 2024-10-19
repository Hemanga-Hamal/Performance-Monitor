// custom libraries
#include "raylib.h"
#include "BarV1.h"
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

    InitWindow(screenWidth, screenHeight, "Performance Monitor - GraphicV1");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(300, 300);
    SetTargetFPS(60);

    int frameCounter = 0;

    // Gauge instances
    GaugeV1 gaugeArc(GaugeV1::Theme(), GaugeV1::Dimensions(), GaugeV1::Config::ConfigArc());
    GaugeV1 gaugeQuarter(GaugeV1::Theme(), GaugeV1::Dimensions(), GaugeV1::Config::ConfigQuarter());

    // Bar instances
    BarV1::Theme barTheme;
    barTheme.barBackgroundColor = DARKGRAY;
    barTheme.barForegroundColor = GREEN;
    barTheme.textColor = WHITE;

    BarV1::Dimensions barDims;
    barDims.barWidth = 200;
    barDims.barHeight = 30;
    barDims.scalingRatio = 1.0f;
    barDims.textSizeRatio = 0.7f;
    barDims.minSize = 90;
    barDims.maxSize = 350;

    BarV1::Config barConfig(0.0f, 100.0f);
    barConfig.autoScale = true;
    barConfig.screenSizeRatio = 0.25f;

    BarV1 bar1(barTheme, barDims, barConfig);
    BarV1 bar2(barTheme, barDims, barConfig);

    // Custom gauge config
    GaugeV1::Config gaugeConfig;
    gaugeConfig.startAngle = 10.0f;
    gaugeConfig.totalAngle = 10.0f;
    gaugeConfig.autoScale = true;
    gaugeConfig.screenSizeRatio = 0.3f;
    gaugeConfig.method = 1;

    GaugeV1 gaugeTest(GaugeV1::Theme(), GaugeV1::Dimensions(), gaugeConfig);

    // Test value
    float testValue = 0.0f;

    while (!WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // Coordinates for each quadrant
        Vector2 quad1 = { screenWidth * 0.25f, screenHeight * 0.25f };
        Vector2 quad2 = { screenWidth * 0.75f, screenHeight * 0.25f };
        Vector2 quad3 = { screenWidth * 0.25f, screenHeight * 0.75f };
        Vector2 quad4 = { screenWidth * 0.75f, screenHeight * 0.75f };

        // Update values
        updateMetrics(testValue, frameCounter);
        gaugeArc.setValue(testValue);
        gaugeQuarter.setValue(testValue);
        
        // Update bar configurations with new test value
        BarV1::Config newConfig1 = bar1.getConfig();
        newConfig1.value = testValue;
        bar1.setConfig(newConfig1);

        BarV1::Config newConfig2 = bar2.getConfig();
        newConfig2.value = std::min(100.0f - testValue, 100.0f); // Inverse of first bar
        bar2.setConfig(newConfig2);

        // Start drawing
        BeginDrawing();
        {
            ClearBackground(BLACK);

            // Draw quadrant lines
            DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE);
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

            // Draw components in different quadrants
            gaugeArc.draw(quad1, "Gauge Arc");
            gaugeQuarter.draw(quad2, "Gauge Quarter");
            
            // Draw bars
            bar1.draw(quad3, "Bar Test 1", std::to_string(static_cast<int>(testValue)));
            bar2.draw(quad4, "Bar Test 2", std::to_string(static_cast<int>(100.0f - testValue)));

            // Debug points for quadrant centers
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