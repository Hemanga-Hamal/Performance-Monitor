#include "BarV1.h"

// Theme Configuration
BarV1::Theme::Theme() 
    : barBackgroundColor(DARKGRAY), barForegroundColor(GRAY), textColor(WHITE) {}

// Dimension Configuration
BarV1::Dimensions::Dimensions()
    : barWidth(300), barHeight(40), scalingRatio(1.0f), textSizeRatio(0.7f),
      minSize(90), maxSize(350) {}

// Configuration for Bar Values
BarV1::Config::Config()
    : value(0), maxValue(100), autoScale(true), screenSizeRatio(0.25f) {}

BarV1::Config::Config(float val, float maxVal)
    : value(val), maxValue(maxVal), autoScale(true), screenSizeRatio(0.25f) {}

// Constructor for BarV1
BarV1::BarV1(const Theme& theme, const Dimensions& dimensions, const Config& config) 
    : theme(theme), dims(dimensions), config(config) {}

// Calculates the bar size based on screen size and scaling ratios
float BarV1::calculateBarSize() const {
    if (config.autoScale) {
        float screenSize = std::max(GetScreenWidth(), GetScreenHeight());
        return screenSize * config.screenSizeRatio * dims.scalingRatio;
    }
    return dims.barWidth * dims.scalingRatio;
}

// Draws the bar on the screen
void BarV1::draw(Vector2 centre, const std::string& label, const std::string& numb) const {
    // Calculate value percentage and bar size
    float valuePercentage = config.value / config.maxValue;
    float barSize = calculateBarSize();

    // Clamp the bar size between minSize and maxSize
    barSize = std::clamp(barSize, dims.minSize, dims.maxSize);

    // Scale dimensions for rendering
    float scaledWidth = barSize;
    float scaledHeight = barSize * (dims.barHeight / dims.barWidth);

    // Determine position for rectangle drawing
    Vector2 upperLeft = { centre.x - scaledWidth / 2, centre.y - scaledHeight / 2 + scaledHeight/2 +2.5f};

    // Draw background and foreground bars
    DrawRectangle(static_cast<int>(upperLeft.x), static_cast<int>(upperLeft.y), 
                  static_cast<int>(scaledWidth), static_cast<int>(scaledHeight), theme.barBackgroundColor);
    DrawRectangle(static_cast<int>(upperLeft.x), static_cast<int>(upperLeft.y), 
                  static_cast<int>(scaledWidth * valuePercentage), static_cast<int>(scaledHeight), theme.barForegroundColor);

    // Font size calculation based on bar dimensions
    float fontSize = std::max(scaledHeight * dims.textSizeRatio, 9.3f);

    // Apply point filtering for text rendering
    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);

    // Draw label and number above the bar
    Vector2 labelPosition = { upperLeft.x, upperLeft.y - fontSize - 5 };
    DrawTextPro(GetFontDefault(), label.c_str(), labelPosition, { 0, 0 }, 0.0f, fontSize, 2.0f, theme.textColor);

    Vector2 numbSize = MeasureTextEx(GetFontDefault(), numb.c_str(), fontSize, 2.0f);
    Vector2 numbPosition = { upperLeft.x + scaledWidth - numbSize.x, upperLeft.y - fontSize - 5 };
    DrawTextPro(GetFontDefault(), numb.c_str(), numbPosition, { 0, 0 }, 0.0f, fontSize, 2.0f, theme.textColor);
}


// Setters for theme, dimensions, and configuration
void BarV1::setTheme(const Theme& newTheme) {
    theme = newTheme;
}

void BarV1::setDimensions(const Dimensions& newDimensions) {
    dims = newDimensions;
}

void BarV1::setConfig(const Config& newConfig) {
    config = newConfig;
}

void BarV1::setValue(float value){
    config.value = value;
}

// Getters for retrieving configuration and dimensions
const BarV1::Config& BarV1::getConfig() const {
    return config;
}

float BarV1::getWidth() const {
    return calculateBarSize();
}

float BarV1::getHeight() const {
    return calculateBarSize() * (dims.barHeight / dims.barWidth);
}
