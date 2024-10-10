#include "Bar.h"

Bar::Theme::Theme() 
    : barBackgroundColor(DARKGRAY), barForegroundColor(GRAY) {}

Bar::Dimensions::Dimensions() 
    : barWidth(400), barHeight(30), scalingRatio(1.0f) {}

Bar::Config::Config() 
    : value(0), maxValue(100) {}

Bar::Config::Config(float val, float maxVal) 
    : value(val), maxValue(maxVal) {}

Bar::Bar(const Theme& theme, const Dimensions& dimensions, const Config& config) 
    : theme(theme), dims(dimensions), config(config) {}

void Bar::draw(Vector2 position) const {
    float valuePercentage = config.value / config.maxValue;

    DrawRectangle((int)position.x, (int)position.y, (int)dims.barWidth, (int)dims.barHeight, theme.barBackgroundColor);
    DrawRectangle((int)position.x, (int)position.y, (int)(dims.barWidth * valuePercentage), (int)dims.barHeight, theme.barForegroundColor);
}

void Bar::setTheme(const Theme& newTheme) {
    theme = newTheme;
}

void Bar::setDimensions(const Dimensions& newDimensions) {
    dims = newDimensions;
}

void Bar::setConfig(const Config& newConfig) {
    config = newConfig;
}

const Bar::Config& Bar::getConfig() const {
    return config;
}

float Bar::getwidth() const {
    return dims.barWidth;
}

float Bar::getheight() const {
    return dims.barHeight;
}
