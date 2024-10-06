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
        Color labelColor;

        Theme(); // Default constructor
    };

    struct Dimensions {
        float baseSize;           // Base size in pixels
        float scaleRatio;         // Scale multiplier
        float arcThickness;       // Thickness of the arc
        float textSizeRatio;      // Text size relative to gauge
        float labelSizeRatio;     // Label size relative to gauge
        float labelOffset;        // Vertical label offset
        float minSize;            // Minimum size threshold
        float maxSize;            // Maximum size threshold

        Dimensions(); // Default constructor
    };

    struct Config {
        float startAngle;         // Start angle in degrees
        float totalAngle;         // Total sweep angle in degrees
        const char* label;        // Label text
        bool autoScale;           // Whether to auto-scale with screen size
        float screenSizeRatio;    // Ratio of screen size for autoScale

        Config(); // Default constructor
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

    void setValue(float newValue); // Set gauge value
    void setScale(float scale);    // Set the scaling factor
    void setBaseSize(float size);  // Set base size of the gauge
    void setArcThickness(float thickness); // Set thickness of the arc

    float calculateGaugeSize() const; // Calculate the size of the gauge
    void draw(Vector2 center) const;  // Draw the gauge
};

#endif // GAUGE_H
