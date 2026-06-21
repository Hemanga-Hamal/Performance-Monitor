#ifndef GAUGE_H
#define GAUGE_H

#include "raylib.h"
#include <string>
#include <cmath>

class Gauge {
public:
    static float clamp(float value, float min, float max);

private:
    float DegToRad(float degrees) const;

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
        int method;

        Config();
    
        // Static method for an arc configuration
        static Config ConfigArc() {
            Config config;
            config.startAngle = 150.0f;
            config.totalAngle = 240.0f;
            config.autoScale = true;
            config.screenSizeRatio = 0.3f;
            config.method = 1;
            return config;
        }

        // Static method for a quarter configuration
        static Config ConfigQuarter() {
            Config config;
            config.startAngle = 90.0f;
            config.totalAngle = 270.0f;
            config.autoScale = true;
            config.screenSizeRatio = 0.3f;
            config.method = 2;
            return config;
        }

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
    void setTheme(const Theme& newTheme);
    void setTotalAngle(float angle) noexcept;
    void setStartAngle(float angle) noexcept;
    void setAutoScale(bool autoScale) noexcept;
    void setScreenSizeRatio(float ratio) noexcept;
    void setTextColor(Color color) noexcept;

    float calculateGaugeSize() const;
    void draw(Vector2 center, const std::string& label) const;
    void drawInRect(Rectangle bounds, const std::string& label) const;
};

#endif // GAUGE_H
