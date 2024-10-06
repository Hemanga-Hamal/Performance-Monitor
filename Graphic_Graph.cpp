#include "raylib.h"

int main() {
    // Initial screen size
    int screenWidth = 800;
    int screenHeight = 600;

    // Initialize the window
    InitWindow(screenWidth, screenHeight, "Performance Monitor - > Gauge");

    // Enable window resizing and set minimum size
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(200, 200);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Get current screen size
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // Use explicit casting to float
        Vector2 quad1 = {(float)(screenWidth/4)     , (float)(screenHeight/4)};
        Vector2 quad2 = {(float)((screenWidth/4)*3) , (float)(screenHeight/4)};
        Vector2 quad3 = {(float)(screenWidth/4)     , (float)((screenHeight/4)*3)};
        Vector2 quad4 = {(float)((screenWidth/4)*3) , (float)((screenHeight/4)*3)};

        BeginDrawing();
            ClearBackground(WHITE);


        EndDrawing();
    }

    CloseWindow();
    return 0;
}
