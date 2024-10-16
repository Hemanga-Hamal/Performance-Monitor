#include "raylib.h"
#include "BarV1.h"
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

    InitWindow(screenWidth, screenHeight, "Performance Monitor - BarV2");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(300, 300);
    SetTargetFPS(60);

    // Bar setup
    BarV1::Theme barTheme;
    BarV1::Dimensions barDimensions;
    BarV1::Config barConfig;
    BarV1 testBar(barTheme, barDimensions, barConfig);

    int frameCounter = 0;

    while (!WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // Coordinates
        Vector2 quad1 = { screenWidth * 0.25f, screenHeight * 0.25f };
        Vector2 quad2 = { screenWidth * 0.75f, screenHeight * 0.25f };
        Vector2 quad3 = { screenWidth * 0.25f, screenHeight * 0.75f };
        Vector2 quad4 = { screenWidth * 0.75f, screenHeight * 0.75f };

        // Window scale
        float widthScale = static_cast<float>(screenWidth) / baseWidth;
        float heightScale = static_cast<float>(screenHeight) / baseHeight;
        float overallScale = std::min(widthScale, heightScale);

        // Update Bar
        float test;
        updateMetrics(test, frameCounter);
        
        BarV1::Dimensions newDimensions;
        newDimensions.scalingRatio = overallScale;
        testBar.setDimensions(newDimensions);
        testBar.setConfig(BarV1::Config(test, 100));

        BeginDrawing();
        {   
            ClearBackground(BLACK);

            // Draw quadrant lines
            DrawLine(0, screenHeight / 2, screenWidth, screenHeight / 2, WHITE);
            DrawLine(screenWidth / 2, 0, screenWidth / 2, screenHeight, WHITE);

            // Draw Bars with text
            testBar.draw(quad1, "Test", std::to_string(static_cast<int>(test)));
            testBar.draw(quad2, "CPU", std::to_string(static_cast<int>(test * 0.8f)) + "%");
            testBar.draw(quad3, "RAM", std::to_string(static_cast<int>(test * 1.2f) % 100) + "%");
            testBar.draw(quad4, "NET", std::to_string(static_cast<int>(test * 0.5f)) + " MB/s");

            // Center debug
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