#include "raylib.h"

int main() {
    // Initial screen size
    int screenWidth = 800;
    int screenHeight = 600;

    // Initialize the window
    InitWindow(screenWidth, screenHeight, "Performance Monitor - > Layout");

    // Enable window resizing and set minimum size
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(200, 200);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Get current screen size (if the window is resized, these values will change)
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // Use explicit casting to float
        Vector2 quad1 = {(float)(screenWidth/4)     , (float)(screenHeight/4)};
        Vector2 quad2 = {(float)((screenWidth/4)*3) , (float)(screenHeight/4)};
        Vector2 quad3 = {(float)(screenWidth/4)     , (float)((screenHeight/4)*3)};
        Vector2 quad4 = {(float)((screenWidth/4)*3) , (float)((screenHeight/4)*3)};

        BeginDrawing();
            ClearBackground(BLACK);

            // Quadrants
            DrawLine(0, screenHeight/2, screenWidth, screenHeight/2, WHITE); 
            DrawLine(screenWidth/2, 0, screenWidth/2, screenHeight, WHITE);

            // Texts in each quadrant
            const char* CPU = "CPU";
            const char* RAM = "RAM";
            const char* Network = "Network";
            const char* Disk = "Disk";
            int fontSize = 20;

            // Calculate text width and height for dynamic centering
            int textWidth1 = MeasureText(CPU, fontSize);
            int textWidth2 = MeasureText(RAM, fontSize);
            int textWidth3 = MeasureText(Network, fontSize);
            int textWidth4 = MeasureText(Disk, fontSize);
            int textHeight = fontSize;

            // Draw text centered in each quadrant
            DrawText(CPU    , quad1.x - textWidth1/2    , quad1.y - textHeight/2 - screenHeight/5, fontSize, WHITE);
            DrawText(RAM    , quad2.x - textWidth2/2    , quad2.y - textHeight/2 - screenHeight/5, fontSize, WHITE);
            DrawText(Network, quad3.x - textWidth3/2    , quad3.y - textHeight/2 - screenHeight/5, fontSize, WHITE);
            DrawText(Disk   , quad4.x - textWidth4/2    , quad4.y - textHeight/2 - screenHeight/5, fontSize, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
