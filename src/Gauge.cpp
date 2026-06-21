#include "Gauge.h"
#include <algorithm>

float Gauge::clamp(float value, float min, float max) {
    return std::min(std::max(value, min), max);
}

float Gauge::DegToRad(float degrees) const {
    return degrees * PI / 180.0f;
}

void Gauge::DrawArc(Vector2 center, float innerRadius, float outerRadius, 
                    float startAngle, float endAngle, Color color) const {
    int segments = 300;
    float span = endAngle - startAngle;
    if (span <= 0.0f) return;
    float angleStep = span / segments;

    for (int i = 0; i < segments; i++) {
        float a1 = startAngle + i * angleStep;
        float a2 = a1 + angleStep;

        Vector2 p1i = {center.x + innerRadius * cosf(DegToRad(a1)),
                       center.y + innerRadius * sinf(DegToRad(a1))};
        Vector2 p2i = {center.x + innerRadius * cosf(DegToRad(a2)),
                       center.y + innerRadius * sinf(DegToRad(a2))};
        Vector2 p1o = {center.x + outerRadius * cosf(DegToRad(a1)),
                       center.y + outerRadius * sinf(DegToRad(a1))};
        Vector2 p2o = {center.x + outerRadius * cosf(DegToRad(a2)),
                       center.y + outerRadius * sinf(DegToRad(a2))};

        DrawTriangle(p1i, p2i, p1o, color);
        DrawTriangle(p2i, p2o, p1o, color);
    }
}

Gauge::Theme::Theme() :
    backgroundColor({20, 20, 20, 255}),
    arcBackgroundColor({40, 40, 40, 255}),
    arcActiveColor({150, 150, 150, 255}),
    textColor(WHITE) {}

Gauge::Dimensions::Dimensions() :
    baseSize(200.0f),
    scaleRatio(1.0f),
    arcThickness(0.2f),
    textSizeRatio(0.2f),
    minSize(50.0f),
    maxSize(1000.0f) {}

Gauge::Config::Config() :
    startAngle(150.0f),
    totalAngle(240.0f),
    autoScale(true),
    screenSizeRatio(0.3f),
    method(1) {}

Gauge::Gauge(const Theme& theme, 
             const Dimensions& dimensions, 
             const Config& config) 
    : theme(theme), dims(dimensions), config(config), value(0.0f) {}

void Gauge::setValue(float newValue) { value = clamp(newValue, 0.0f, 100.0f); }
void Gauge::setScale(float scale) { dims.scaleRatio = scale; }
void Gauge::setBaseSize(float size) { dims.baseSize = size; }
void Gauge::setArcThickness(float thickness) { dims.arcThickness = thickness; }
void Gauge::setTotalAngle(float angle) noexcept { config.totalAngle = angle; }
void Gauge::setStartAngle(float angle) noexcept { config.startAngle = angle; }
void Gauge::setAutoScale(bool autoScale) noexcept { config.autoScale = autoScale; }
void Gauge::setScreenSizeRatio(float ratio) noexcept { config.screenSizeRatio = ratio; }
void Gauge::setTheme(const Theme& newTheme) { theme = newTheme; }
void Gauge::setTextColor(Color color) noexcept { theme.textColor = color; }

float Gauge::calculateGaugeSize() const {
    if (config.autoScale) {
        float screenSize = fmin(GetScreenWidth(), GetScreenHeight());
        return screenSize * config.screenSizeRatio * dims.scaleRatio;
    }
    return dims.baseSize * dims.scaleRatio;
}

void Gauge::draw(Vector2 center, const std::string& label) const {
    float gaugeSize = calculateGaugeSize();
    gaugeSize = clamp(gaugeSize, dims.minSize, dims.maxSize);

    float screenMin = std::min(GetScreenWidth(), GetScreenHeight());
    if (gaugeSize > screenMin * 0.9f) gaugeSize = screenMin * 0.9f;

    float outerRadius = gaugeSize / 2;
    float innerRadius = outerRadius * (1.0f - dims.arcThickness);
    float midRadius = (innerRadius + outerRadius) / 2;
    float endAngle = config.startAngle + config.totalAngle;
    float loadAngle = config.startAngle + (value / 100.0f) * config.totalAngle;

    DrawArc(center, innerRadius, outerRadius, config.startAngle, endAngle, theme.arcBackgroundColor);

    if (value > 0.5f) {
        float glowInner = innerRadius * 0.97f;
        float glowOuter = outerRadius * 1.03f;
        DrawArc(center, glowInner, glowOuter, config.startAngle, loadAngle, 
                {theme.arcActiveColor.r, theme.arcActiveColor.g, theme.arcActiveColor.b, 30});
    }

    if (value > 0) {
        DrawArc(center, innerRadius, outerRadius, config.startAngle, loadAngle, theme.arcActiveColor);

        float capR = (outerRadius - innerRadius) * 0.5f;
        if (capR > 6.0f) capR = 6.0f;
        if (capR > 1.0f) {
            Vector2 capPos = {center.x + midRadius * cosf(DegToRad(loadAngle)),
                              center.y + midRadius * sinf(DegToRad(loadAngle))};
            DrawCircleV(capPos, capR, theme.arcActiveColor);
        }
    }

    float valueFontSize = gaugeSize * dims.textSizeRatio * dims.scaleRatio;
    float labelFontSize = valueFontSize * 0.48f;
    if (labelFontSize < 8.0f) labelFontSize = 8.0f;
    
    const char* valueText = TextFormat("%.0f%%", value);
    Vector2 valueTextSize = MeasureTextEx(GetFontDefault(), valueText, valueFontSize, 1);
    DrawText(valueText,
             center.x - valueTextSize.x / 2,
             center.y - valueTextSize.y / 2,
             valueFontSize,
             theme.textColor);

    if (!label.empty()) {
        Vector2 labelTextSize = MeasureTextEx(GetFontDefault(), label.c_str(), labelFontSize, 1);

        if (config.method == 1) {
            DrawText(label.c_str(),
                    center.x - labelTextSize.x / 2,
                    center.y + valueTextSize.y / 2 + innerRadius/10,
                    labelFontSize,
                    theme.textColor);
        }
        else if (config.method == 2) {
            DrawText(label.c_str(),
                    center.x + innerRadius/10,
                    center.y + (outerRadius - innerRadius)/2 + innerRadius - labelTextSize.y / 2.1f,
                    labelFontSize,
                    theme.textColor);
        }
    }
}

void Gauge::drawInRect(Rectangle bounds, const std::string& label) const {
    float maxRadius = std::min(bounds.width * 0.42f, bounds.height * 0.42f);
    if (maxRadius > dims.maxSize / 2.0f) maxRadius = dims.maxSize / 2.0f;
    if (maxRadius < 20.0f) maxRadius = 20.0f;
    Vector2 center = { bounds.x + bounds.width / 2, bounds.y + bounds.height * 0.45f };

    float outerRadius = maxRadius;
    float innerRadius = outerRadius * (1.0f - dims.arcThickness);
    float midRadius = (innerRadius + outerRadius) / 2;
    float endAngle = config.startAngle + config.totalAngle;
    float loadAngle = config.startAngle + (value / 100.0f) * config.totalAngle;

    DrawArc(center, innerRadius, outerRadius, config.startAngle, endAngle, theme.arcBackgroundColor);

    if (value > 0.5f) {
        float glowInner = innerRadius * 0.97f;
        float glowOuter = outerRadius * 1.03f;
        DrawArc(center, glowInner, glowOuter, config.startAngle, loadAngle, 
                {theme.arcActiveColor.r, theme.arcActiveColor.g, theme.arcActiveColor.b, 30});
    }

    if (value > 0) {
        DrawArc(center, innerRadius, outerRadius, config.startAngle, loadAngle, theme.arcActiveColor);

        float capR = (outerRadius - innerRadius) * 0.5f;
        if (capR > 6.0f) capR = 6.0f;
        if (capR > 1.0f) {
            Vector2 capPos = {center.x + midRadius * cosf(DegToRad(loadAngle)),
                              center.y + midRadius * sinf(DegToRad(loadAngle))};
            DrawCircleV(capPos, capR, theme.arcActiveColor);
        }
    }

    float valueFontSize = outerRadius * 0.35f;
    float labelFontSize = valueFontSize * 0.48f;
    if (labelFontSize < 8.0f) labelFontSize = 8.0f;

    const char* valueText = TextFormat("%.0f%%", value);
    Vector2 valueTextSize = MeasureTextEx(GetFontDefault(), valueText, valueFontSize, 1);
    DrawText(valueText,
             center.x - valueTextSize.x / 2,
             center.y - valueTextSize.y / 2,
             valueFontSize,
             theme.textColor);

    if (!label.empty()) {
        Vector2 labelTextSize = MeasureTextEx(GetFontDefault(), label.c_str(), labelFontSize, 1);
        DrawText(label.c_str(),
                 center.x - labelTextSize.x / 2,
                 center.y + valueTextSize.y / 2 + innerRadius / 10,
                 labelFontSize,
                 theme.textColor);
    }
}
