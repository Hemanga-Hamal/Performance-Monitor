#ifndef THEMEV1_H
#define THEMEV1_H

#include "raylib.h"

struct ThemeV1 {
    Color windowBg;
    Color tileBg;
    Color tileBorder;
    Color titleText;
    Color textPrimary;
    Color textSecondary;
    Color lineColor;
    Color barBackground;
    Color barForeground;
    Color gaugeArcBg;
    Color gaugeArcActive;
    Color gaugeText;
    Color landingBg;

    static ThemeV1 Dark() {
        ThemeV1 t;
        t.windowBg = {18, 18, 22, 255};
        t.tileBg = {28, 28, 36, 255};
        t.tileBorder = {50, 50, 60, 255};
        t.titleText = {220, 220, 230, 255};
        t.textPrimary = {200, 200, 210, 255};
        t.textSecondary = {140, 140, 150, 255};
        t.lineColor = {60, 60, 70, 255};
        t.barBackground = {50, 50, 55, 255};
        t.barForeground = {80, 160, 220, 255};
        t.gaugeArcBg = {40, 40, 45, 255};
        t.gaugeArcActive = {80, 160, 220, 255};
        t.gaugeText = {220, 220, 230, 255};
        t.landingBg = {14, 14, 18, 255};
        return t;
    }

    static ThemeV1 Light() {
        ThemeV1 t;
        t.windowBg = {235, 235, 240, 255};
        t.tileBg = {248, 248, 252, 255};
        t.tileBorder = {200, 200, 210, 255};
        t.titleText = {30, 30, 40, 255};
        t.textPrimary = {40, 40, 50, 255};
        t.textSecondary = {120, 120, 130, 255};
        t.lineColor = {210, 210, 220, 255};
        t.barBackground = {220, 220, 225, 255};
        t.barForeground = {60, 130, 210, 255};
        t.gaugeArcBg = {210, 210, 215, 255};
        t.gaugeArcActive = {60, 130, 210, 255};
        t.gaugeText = {30, 30, 40, 255};
        t.landingBg = {225, 225, 230, 255};
        return t;
    }

    static ThemeV1 HighContrast() {
        ThemeV1 t;
        t.windowBg = {0, 0, 0, 255};
        t.tileBg = {10, 10, 10, 255};
        t.tileBorder = {255, 255, 255, 255};
        t.titleText = {255, 255, 0, 255};
        t.textPrimary = {255, 255, 255, 255};
        t.textSecondary = {180, 180, 180, 255};
        t.lineColor = {255, 255, 255, 255};
        t.barBackground = {30, 30, 30, 255};
        t.barForeground = {0, 255, 255, 255};
        t.gaugeArcBg = {20, 20, 20, 255};
        t.gaugeArcActive = {0, 255, 0, 255};
        t.gaugeText = {255, 255, 255, 255};
        t.landingBg = {0, 0, 0, 255};
        return t;
    }
};

#endif
