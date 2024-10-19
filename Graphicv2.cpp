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
    // window initialization
    const int baseWidth = 800;
    const int baseHeight = 600;
    int screenWidth = baseWidth;
    int screenHeight = baseHeight;

    InitWindow(screenWidth, screenHeight, "Performance Monitor - GraphicV2 -> dot notation");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(300, 300);
    SetTargetFPS(60);

    int frameCounter = 0;

    // gauge instances
    GaugeV1 CPU_Util(GaugeV1::Theme(), GaugeV1::Dimensions(), GaugeV1::Config::ConfigArc());
    GaugeV1 RAM_Util(GaugeV1::Theme(), GaugeV1::Dimensions(), GaugeV1::Config::ConfigQuarter());

    // bar instancees
    BarV1::Theme Bar_Themes;
    BarV1::Dimensions Bar_Dimesions;
    BarV1::Config Bar_config;

    BarV1 CPU_Freq(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Wifi_Send(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Wifi_Recv(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Ether_Send(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Ether_Recv(Bar_Themes, Bar_Dimesions, Bar_config);
    BarV1 Disk_Util(Bar_Themes, Bar_Dimesions, Bar_config);

    //test value
    float testValue = 0.0f;

    while (!WindowShouldClose()){
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        // coordinates for each quadrant
        Vector2 quad1 = { screenWidth * 0.25f, screenHeight * 0.25f };
        Vector2 quad2 = { screenWidth * 0.75f, screenHeight * 0.25f };
        Vector2 quad3 = { screenWidth * 0.25f, screenHeight * 0.75f };
        Vector2 quad4 = { screenWidth * 0.75f, screenHeight * 0.75f };

        //update values
        updateMetrics(testValue, frameCounter);
        
        CPU_Util.setValue(testValue);
        RAM_Util.setValue(testValue);
        
        CPU_Freq.setValue(testValue); 
        Wifi_Send.setValue(testValue);
        Wifi_Recv.setValue(testValue);
        Ether_Send.setValue(testValue);
        Ether_Recv.setValue(testValue);


        //Drawing
        BeginDrawing();
        {

        }
        EndDrawing();
        frameCounter++;
    }


    CloseWindow();
    return 0;
}