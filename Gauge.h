#ifndef GAUGE_H
#define GAUGE_H

#include "raylib.h"
#include <cmath>
#include <algorithm>

class Gauge {
private:
    static float clamp(float value, float min, float max); // Helper function

    float DegToRad(float degrees) const; // Convert degrees to radians

    void DrawArc(Vector2 center, float innerRadius, float outerRadius, 
                 float startAngle, float endAngle, Color color) const; // Draw arc

public:
    struct Theme {
        Color backgroundColor;
        Color arcBackgroundColor;
        Color arcActiveColor;
        Color textColor;

        Theme();
    };

    struct Dimensions {
        float baseSize;           // Base size in pixels
        float scaleRatio;         // Scale multiplier
        float arcThickness;       // Thickness of the arc
        float textSizeRatio;      // Text size relative to gauge
        float minSize;            // Minimum size threshold
        float maxSize;            // Maximum size threshold

        Dimensions();
    };

    struct Config {
        float startAngle;         // Start angle in degrees
        float totalAngle;         // Total sweep angle in degrees
        bool autoScale;
        float screenSizeRatio;

        Config();
    };

private:
    Theme theme;
    Dimensions dims;
    Config config;
    float value;

public:
    Gauge(const Theme& theme = Theme(), 
          const Dimensions& dimensions = Dimensions(),
          const Config& config = Config());

    void setValue(float newValue);
    void setScale(float scale);
    void setBaseSize(float size);
    void setArcThickness(float thickness);

    float calculateGaugeSize() const;
    void draw(Vector2 center) const;
};

#endif
