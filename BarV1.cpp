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
        // Use the smallest screen dimension to maintain proportions
        float screenSize = std::min(GetScreenWidth(), GetScreenHeight());
        float baseSize = screenSize * config.screenSizeRatio * dims.scalingRatio;
        return std::clamp(baseSize, static_cast<float>(dims.minSize), static_cast<float>(dims.maxSize));
    }
    return std::clamp(dims.barWidth * dims.scalingRatio, 
                     static_cast<float>(dims.minSize), 
                     static_cast<float>(dims.maxSize));
}

// Draws the bar on the screen
void BarV1::draw(Vector2 centre, const std::string& label, const std::string& numb) const {
    // Calculate value percentage and bar size
    float valuePercentage = std::clamp(config.value / config.maxValue, 0.0f, 1.0f);
    float barSize = calculateBarSize();

    // Calculate base dimensions while maintaining aspect ratio
    float aspectRatio = static_cast<float>(dims.barHeight) / dims.barWidth;
    float baseWidth = barSize;
    float baseHeight = baseWidth * aspectRatio;

    // Adjust dimensions if height would exceed screen bounds
    float maxAllowedHeight = GetScreenHeight() * 0.1f; // Limit height to 10% of screen height
    if (baseHeight > maxAllowedHeight) {
        baseHeight = maxAllowedHeight;
        baseWidth = baseHeight / aspectRatio;
    }

    // Ensure minimum dimensions
    float scaledWidth = std::max(baseWidth, static_cast<float>(dims.minSize));
    float scaledHeight = std::max(baseHeight, dims.minSize * aspectRatio);

    // Calculate bar position
    Vector2 upperLeft = {
        centre.x - scaledWidth / 2,
        centre.y - scaledHeight / 2 + scaledHeight/2 + 2.5f
    };

    // Draw background bar
    DrawRectangle(
        static_cast<int>(upperLeft.x),
        static_cast<int>(upperLeft.y),
        static_cast<int>(scaledWidth),
        static_cast<int>(scaledHeight),
        theme.barBackgroundColor
    );

    // Draw foreground (progress) bar
    DrawRectangle(
        static_cast<int>(upperLeft.x),
        static_cast<int>(upperLeft.y),
        static_cast<int>(scaledWidth * valuePercentage),
        static_cast<int>(scaledHeight),
        theme.barForegroundColor
    );

    // Calculate font size based on bar height while ensuring minimum readability
    float minFontSize = 10.0f;
    float fontSize = std::max(scaledHeight * dims.textSizeRatio, minFontSize);

    // Apply point filtering for crisp text
    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);

    // Calculate text positions
    float textY = upperLeft.y - fontSize - 5;
    
    // Draw label
    Vector2 labelPosition = { upperLeft.x, textY };
    DrawTextPro(
        GetFontDefault(),
        label.c_str(),
        labelPosition,
        Vector2{ 0, 0 },
        0.0f,
        fontSize,
        2.0f,
        theme.textColor
    );

    // Draw number (right-aligned)
    Vector2 numbSize = MeasureTextEx(GetFontDefault(), numb.c_str(), fontSize, 2.0f);
    Vector2 numbPosition = {
        upperLeft.x + scaledWidth - numbSize.x,
        textY
    };
    DrawTextPro(
        GetFontDefault(),
        numb.c_str(),
        numbPosition,
        Vector2{ 0, 0 },
        0.0f,
        fontSize,
        2.0f,
        theme.textColor
    );
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

void BarV1::setValue(float value) {
    config.value = value;
}

// Getters for retrieving configuration and dimensions
const BarV1::Config& BarV1::getConfig() const {
    return config;
}

float BarV1::getWidth() const {
    float barSize = calculateBarSize();
    float aspectRatio = static_cast<float>(dims.barHeight) / dims.barWidth;
    float maxAllowedHeight = GetScreenHeight() * 0.1f;
    float baseHeight = barSize * aspectRatio;
    
    if (baseHeight > maxAllowedHeight) {
        return maxAllowedHeight / aspectRatio;
    }
    return barSize;
}

float BarV1::getHeight() const {
    float barSize = calculateBarSize();
    float aspectRatio = static_cast<float>(dims.barHeight) / dims.barWidth;
    float height = barSize * aspectRatio;
    return std::min(height, GetScreenHeight() * 0.1f);
}