#ifndef GaugeV1_H
#define GaugeV1_H

#include "raylib.h"
#include <cmath>
#include <algorithm>

class GaugeV1 {
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
    
        // Static method for an arc configuration
        static Config ConfigArc() {
            Config config;
            config.startAngle = 150.0f;
            config.totalAngle = 240.0f;
            config.autoScale = true;
            config.screenSizeRatio = 0.3f;
            return config;
        }

        // Static method for a quarter configuration
        static Config ConfigQuarter() {
            Config config;
            config.startAngle = 135.0f;
            config.totalAngle = 270.0f;
            config.autoScale = true;
            config.screenSizeRatio = 0.3f;
            return config;
        }

    };

private:
    Theme theme;
    Dimensions dims;
    Config config;
    float value;

public:
    GaugeV1(const Theme& theme = Theme(), 
           const Dimensions& dimensions = Dimensions(),
           const Config& config = Config());

    void setValue(float newValue);
    void setScale(float scale);
    void setBaseSize(float size);
    void setArcThickness(float thickness);
    void setTotalAngle(float angle);
    void setStartAngle(float angle);
    void setAutoScale(bool autoScale);
    void setScreenSizeRatio(float ratio);
    void setTextColor(Color color);

    float calculateGaugeSize() const;
    void draw(Vector2 center) const;
};

#endif // GaugeV1_H