#ifndef GaugeV1_H
#define GaugeV1_H

#include "raylib.h"

class GaugeV1 {
public:
    // Theme
    struct Theme {
        Color backgroundColor;
        Color arcBackgroundColor;
        Color arcActiveColor;
        Color textColor;

        Theme();
    };

    // Dimensions
    struct Dimensions {
        float baseSize;
        float scaleRatio;
        float arcThickness;
        float textSizeRatio;
        float minSize;
        float maxSize;

        Dimensions();
    };

    // Config
    struct Config {
        float startAngle;
        float totalAngle;
        bool autoScale;
        float screenSizeRatio;

        Config();
    };

    //constructor
    GaugeV1(const Theme& theme = Theme(),
           const Dimensions& dimensions = Dimensions(),
           const Config& config = Config());

    //setters
    void setValue(float newValue);
    void setScale(float scale);
    void setBaseSize(float size);
    void setArcThickness(float thickness);

    //getters
    float calculateGaugeSize() const;

    //draw
    void draw(Vector2 center) const;

private:
    Theme theme;
    Dimensions dims;
    Config config;
    float value;
};

#endif