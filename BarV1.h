#ifndef BARV1_H
#define BARV1_H

#include "raylib.h"
#include <string>
#include <algorithm>
#include <cmath>

class BarV1 {
public:
    struct Theme {
        Color barBackgroundColor;
        Color barForegroundColor;
        Color textColor;
        Theme();
    };

    struct Dimensions {
        float barWidth;
        float barHeight;
        float scalingRatio;
        float textSizeRatio;
        float minSize;
        float maxSize;
        Dimensions();
    };

    struct Config {
        float value;
        float maxValue;
        bool autoScale;
        float screenSizeRatio;
        float minBarWidth;  // Add minimum bar width
        Config();
        Config(float val, float maxVal);
    };

    BarV1(const Theme& theme, const Dimensions& dimensions, const Config& config);

    void draw(Vector2 centre, const std::string& label, const std::string& numb) const;

    void setTheme(const Theme& newTheme);
    void setDimensions(const Dimensions& newDimensions);
    void setConfig(const Config& newConfig);

    const Config& getConfig() const;
    float getWidth() const;
    float getHeight() const;

private:
    Theme theme;
    Dimensions dims;
    Config config;

    float calculateBarSize() const; 
};

#endif
