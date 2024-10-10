#ifndef BAR_H
#define BAR_H

#include "raylib.h"

class Bar {
public:
    struct Theme {
        Color barBackgroundColor;
        Color barForegroundColor;

        Theme();
    };

    struct Dimensions {
        float barWidth;
        float barHeight;
        float scalingRatio;

        Dimensions();
    };

    struct Config {
        float value;
        float maxValue;

        Config();
        Config(float val, float maxVal);
    };

    Bar(const Theme& theme = Theme(), 
        const Dimensions& dimensions = Dimensions(),
        const Config& config = Config());

    void draw(Vector2 position) const;
    void setTheme(const Theme& newTheme);
    void setDimensions(const Dimensions& newDimensions);
    void setConfig(const Config& newConfig);
    const Config& getConfig() const;
    float getwidth() const;
    float getheight() const;

private:
    Theme theme;
    Dimensions dims;
    Config config;
};

#endif
