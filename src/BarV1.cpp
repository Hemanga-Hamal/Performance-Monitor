#include "BarV1.h"

BarV1::Theme::Theme() 
    : barBackgroundColor(DARKGRAY), barForegroundColor(GRAY), textColor(WHITE) {}

BarV1::Dimensions::Dimensions()
    : barWidth(300), barHeight(40), scalingRatio(1.0f), textSizeRatio(0.7f),
      minSize(90), maxSize(350) {}

BarV1::Config::Config()
    : value(0), maxValue(100), autoScale(true), screenSizeRatio(0.25f) {}

BarV1::Config::Config(float val, float maxVal)
    : value(val), maxValue(maxVal), autoScale(true), screenSizeRatio(0.25f) {}

BarV1::BarV1(const Theme& theme, const Dimensions& dimensions, const Config& config) 
    : theme(theme), dims(dimensions), config(config) {}

float BarV1::calculateBarSize() const {
    if (config.autoScale) {
        float screenSize = std::min(GetScreenWidth(), GetScreenHeight());
        float baseSize = screenSize * config.screenSizeRatio * dims.scalingRatio;
        return std::clamp(baseSize, static_cast<float>(dims.minSize), static_cast<float>(dims.maxSize));
    }
    return std::clamp(dims.barWidth * dims.scalingRatio, 
                     static_cast<float>(dims.minSize), 
                     static_cast<float>(dims.maxSize));
}

void BarV1::draw(Vector2 centre, const std::string& label, const std::string& numb) const {
    float valuePercentage = std::clamp(config.value / config.maxValue, 0.0f, 1.0f);
    float barSize = calculateBarSize();

    float aspectRatio = static_cast<float>(dims.barHeight) / dims.barWidth;
    float baseWidth = barSize;
    float baseHeight = baseWidth * aspectRatio;

    if (config.autoScale) {
        float maxAllowedHeight = GetScreenHeight() * 0.1f;
        if (baseHeight > maxAllowedHeight) {
            baseHeight = maxAllowedHeight;
            baseWidth = baseHeight / aspectRatio;
        }
    }

    float scaledWidth = std::max(baseWidth, static_cast<float>(dims.minSize));
    float scaledHeight = std::max(baseHeight, dims.minSize * aspectRatio);

    float barX = centre.x - scaledWidth / 2;
    float barY = centre.y - scaledHeight / 2;

    Rectangle bgRect = {barX, barY, scaledWidth, scaledHeight};
    float radius = scaledHeight * 0.35f;
    if (radius > 8.0f) radius = 8.0f;
    if (radius < 2.0f) radius = 2.0f;
    int segments = 12;

    DrawRectangleRounded(bgRect, radius / scaledHeight, segments, theme.barBackgroundColor);

    if (valuePercentage > 0.001f) {
        float fillW = scaledWidth * valuePercentage;
        if (fillW < radius * 2.0f) fillW = radius * 2.0f;
        Rectangle fillRect = {barX, barY, fillW, scaledHeight};
        DrawRectangleRounded(fillRect, radius / scaledHeight, segments, theme.barForegroundColor);
    }

    float minFontSize = 10.0f;
    float fontSize = std::max(scaledHeight * dims.textSizeRatio, minFontSize);
    if (fontSize > scaledHeight * 1.2f) fontSize = scaledHeight * 1.2f;

    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);

    float textY = barY - fontSize - 6;
    if (textY < 4.0f) textY = 4.0f;

    Vector2 numbSize = MeasureTextEx(GetFontDefault(), numb.c_str(), fontSize, 2.0f);
    float availLabelW = scaledWidth - numbSize.x - 16.0f;

    std::string displayLabel = label;
    float labelW = MeasureTextEx(GetFontDefault(), displayLabel.c_str(), fontSize, 2.0f).x;
    if (labelW > availLabelW && availLabelW > 20.0f) {
        while (displayLabel.size() > 4 && labelW > availLabelW) {
            displayLabel = displayLabel.substr(0, displayLabel.size() - 4) + "...";
            labelW = MeasureTextEx(GetFontDefault(), displayLabel.c_str(), fontSize, 2.0f).x;
        }
    }

    Vector2 labelPosition = { barX, textY };
    DrawTextPro(GetFontDefault(), displayLabel.c_str(), labelPosition,
                Vector2{0, 0}, 0.0f, fontSize, 2.0f, theme.textColor);

    Vector2 numbPosition = { barX + scaledWidth - numbSize.x, textY };
    DrawTextPro(GetFontDefault(), numb.c_str(), numbPosition,
                Vector2{0, 0}, 0.0f, fontSize, 2.0f, theme.textColor);
}

void BarV1::setTheme(const Theme& newTheme) { theme = newTheme; }
void BarV1::setDimensions(const Dimensions& newDimensions) { dims = newDimensions; }
void BarV1::setConfig(const Config& newConfig) { config = newConfig; }
void BarV1::setValue(float value) { config.value = value; }

void BarV1::drawInRect(Rectangle bounds, const std::string& label, const std::string& numb) const {
    float valuePercentage = std::clamp(config.value / config.maxValue, 0.0f, 1.0f);
    float barWidth = bounds.width * 0.88f;
    float barHeight = bounds.height * 0.22f;
    if (barHeight > 40.0f) barHeight = 40.0f;
    if (barHeight < 12.0f) barHeight = 12.0f;

    float barX = bounds.x + (bounds.width - barWidth) / 2;
    float barY = bounds.y + bounds.height * 0.55f;

    Rectangle bgRect = {barX, barY, barWidth, barHeight};
    float radius = barHeight * 0.35f;
    if (radius > 8.0f) radius = 8.0f;
    if (radius < 2.0f) radius = 2.0f;
    int segments = 12;

    DrawRectangleRounded(bgRect, radius / barHeight, segments, theme.barBackgroundColor);

    if (valuePercentage > 0.001f) {
        float fillW = barWidth * valuePercentage;
        if (fillW < radius * 2.0f) fillW = radius * 2.0f;
        Rectangle fillRect = {barX, barY, fillW, barHeight};
        DrawRectangleRounded(fillRect, radius / barHeight, segments, theme.barForegroundColor);
    }

    float minFontSize = 10.0f;
    float fontSize = std::max(barHeight * dims.textSizeRatio, minFontSize);

    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);

    float textY = barY - fontSize - 6;
    Vector2 labelPosition = { barX, textY };
    DrawTextPro(GetFontDefault(), label.c_str(), labelPosition,
                Vector2{0, 0}, 0.0f, fontSize, 2.0f, theme.textColor);

    Vector2 numbSize = MeasureTextEx(GetFontDefault(), numb.c_str(), fontSize, 2.0f);
    Vector2 numbPosition = { barX + barWidth - numbSize.x, textY };
    DrawTextPro(GetFontDefault(), numb.c_str(), numbPosition,
                Vector2{0, 0}, 0.0f, fontSize, 2.0f, theme.textColor);
}

float BarV1::getTotalHeight() const {
    float barSize = calculateBarSize();
    float aspectRatio = static_cast<float>(dims.barHeight) / dims.barWidth;
    float baseHeight = barSize * aspectRatio;
    float scaledHeight = std::max(baseHeight, dims.minSize * aspectRatio);
    float fontSize = std::max(scaledHeight * dims.textSizeRatio, 10.0f);
    if (fontSize > scaledHeight * 1.2f) fontSize = scaledHeight * 1.2f;
    return scaledHeight + fontSize + 10.0f;
}

const BarV1::Config& BarV1::getConfig() const { return config; }

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
