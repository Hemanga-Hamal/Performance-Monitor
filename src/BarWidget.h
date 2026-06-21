#ifndef BARWIDGET_H
#define BARWIDGET_H

#include "raylib.h"
#include "AppTheme.h"
#include <string>
#include <algorithm>

class BarWidget {
public:
    struct Theme {
        Color barBackgroundColor;
        Color barForegroundColor;
        Color barForegroundEndColor;
        Color textColor;
        Theme();
        static Theme fromAppTheme(const ::AppTheme& t);
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
        Config();
        Config(float val, float maxVal);
    };

    BarWidget(const Theme& theme, const Dimensions& dimensions, const Config& config);

    void draw(Vector2 centre, const std::string& label, const std::string& numb) const;
    void drawInRect(Rectangle bounds, const std::string& label, const std::string& numb) const;
    float getTotalHeight() const;

    [[nodiscard]] Rectangle getLastBarRect() const noexcept { return lastBarRect; }
    [[nodiscard]] bool isLabelTruncated() const noexcept { return labelTruncated; }
    [[nodiscard]] const std::string& getLastLabel() const noexcept { return lastLabel; }

    void setTheme(const Theme& newTheme);
    void setDimensions(const Dimensions& newDimensions);
    void setConfig(const Config& newConfig);
    void setValue(float value);
    const Config& getConfig() const;
    float getWidth() const;
    float getHeight() const;

private:
    Theme theme;
    Dimensions dims;
    Config config;
    mutable Rectangle lastBarRect{};
    mutable bool labelTruncated{false};
    mutable std::string lastLabel;

    float calculateBarSize() const; 
};

#endif
