#include "Gauge.h"

// Helper function for clamping values
float Gauge::clamp(float value, float min, float max) {
    return std::min(std::max(value, min), max);
}

// Convert degrees to radians
float Gauge::DegToRad(float degrees) const {
    return degrees * PI / 180.0f;
}

// Draw an arc using triangles
void Gauge::DrawArc(Vector2 center, float innerRadius, float outerRadius, 
                    float startAngle, float endAngle, Color color) const {
    int segments = 300;
    float angleStep = (endAngle - startAngle) / segments;

    for (int i = 0; i < segments; i++) {
        float angle1 = startAngle + i * angleStep;
        float angle2 = angle1 + angleStep;

        Vector2 point1Inner = {
            center.x + innerRadius * cosf(DegToRad(angle1)),
            center.y + innerRadius * sinf(DegToRad(angle1))
        };
        Vector2 point2Inner = {
            center.x + innerRadius * cosf(DegToRad(angle2)),
            center.y + innerRadius * sinf(DegToRad(angle2))
        };
        Vector2 point1Outer = {
            center.x + outerRadius * cosf(DegToRad(angle1)),
            center.y + outerRadius * sinf(DegToRad(angle1))
        };
        Vector2 point2Outer = {
            center.x + outerRadius * cosf(DegToRad(angle2)),
            center.y + outerRadius * sinf(DegToRad(angle2))
        };

        DrawTriangle(point1Inner, point2Inner, point1Outer, color);
        DrawTriangle(point2Inner, point2Outer, point1Outer, color);
    }
}

// Default constructor for Theme struct
Gauge::Theme::Theme() :
    backgroundColor({20, 20, 20, 255}),
    arcBackgroundColor({40, 40, 40, 255}),
    arcActiveColor({150, 150, 150, 255}),
    textColor(WHITE),
    labelColor(GRAY) {}

// Default constructor for Dimensions struct
Gauge::Dimensions::Dimensions() :
    baseSize(200.0f),
    scaleRatio(1.0f),
    arcThickness(0.2f),
    textSizeRatio(0.2f),
    labelSizeRatio(0.1f),
    labelOffset(30.0f),
    minSize(50.0f),
    maxSize(1000.0f) {}

// Default constructor for Config struct
Gauge::Config::Config() :
    startAngle(150.0f),
    totalAngle(240.0f),
    label("Load"),
    autoScale(true),
    screenSizeRatio(0.3f) {}

// Gauge class constructor
Gauge::Gauge(const Theme& theme, 
             const Dimensions& dimensions, 
             const Config& config) 
    : theme(theme), dims(dimensions), config(config), value(0.0f) {}

// Set gauge value
void Gauge::setValue(float newValue) {
    value = clamp(newValue, 0.0f, 100.0f);
}

// Set the scaling factor
void Gauge::setScale(float scale) {
    dims.scaleRatio = std::max(0.1f, scale);
}

// Set the base size of the gauge
void Gauge::setBaseSize(float size) {
    dims.baseSize = clamp(size, dims.minSize, dims.maxSize);
}

// Set the thickness of the arc
void Gauge::setArcThickness(float thickness) {
    dims.arcThickness = clamp(thickness, 0.05f, 0.5f);
}

// Calculate the size of the gauge
float Gauge::calculateGaugeSize() const {
    if (config.autoScale) {
        float screenSize = fmin(GetScreenWidth(), GetScreenHeight());
        return screenSize * config.screenSizeRatio * dims.scaleRatio;
    }
    return dims.baseSize * dims.scaleRatio;
}

// Draw the gauge on the screen
void Gauge::draw(Vector2 center) const {
    float gaugeSize = calculateGaugeSize();
    gaugeSize = clamp(gaugeSize, dims.minSize, dims.maxSize);

    float outerRadius = gaugeSize / 2;
    float innerRadius = outerRadius * (1.0f - dims.arcThickness);
    float endAngle = config.startAngle + config.totalAngle;
    float loadAngle = config.startAngle + (value / 100.0f) * config.totalAngle;

    DrawArc(center, innerRadius, outerRadius, config.startAngle, endAngle, theme.arcBackgroundColor);

    if (value > 0) {
        DrawArc(center, innerRadius, outerRadius, config.startAngle, loadAngle, theme.arcActiveColor);
    }

    float valueFontSize = gaugeSize * dims.textSizeRatio * dims.scaleRatio;
    float labelFontSize = gaugeSize * dims.labelSizeRatio * dims.scaleRatio;

    const char* valueText = TextFormat("%.0f%%", value);
    Vector2 textSize = MeasureTextEx(GetFontDefault(), valueText, valueFontSize, 0);
    DrawText(valueText, 
             center.x - textSize.x / 2, 
             center.y - valueFontSize / 2, 
             valueFontSize, 
             theme.textColor);

    textSize = MeasureTextEx(GetFontDefault(), config.label, labelFontSize, 0);
    DrawText(config.label, 
             center.x - textSize.x / 2, 
             center.y + dims.labelOffset * dims.scaleRatio, 
             labelFontSize, 
             theme.labelColor);
}
