#include "GaugeV1.h"

// Helper function for clamping values
float GaugeV1::clamp(float value, float min, float max) {
    return std::min(std::max(value, min), max);
}

// Convert degrees to radians
float GaugeV1::DegToRad(float degrees) const {
    return degrees * PI / 180.0f;
}

// Draw an arc using triangles
void GaugeV1::DrawArc(Vector2 center, float innerRadius, float outerRadius, 
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
GaugeV1::Theme::Theme() :
    backgroundColor({20, 20, 20, 255}),
    arcBackgroundColor({40, 40, 40, 255}),
    arcActiveColor({150, 150, 150, 255}),
    textColor(WHITE) {}

// Default constructor for Dimensions struct
GaugeV1::Dimensions::Dimensions() :
    baseSize(200.0f),
    scaleRatio(1.0f),
    arcThickness(0.2f),
    textSizeRatio(0.2f),
    minSize(50.0f),
    maxSize(1000.0f) {}

//default constructor for Config struct
GaugeV1::Config::Config() :
    startAngle(150.0f),
    totalAngle(240.0f),
    autoScale(true),
    screenSizeRatio(0.3f),
    method(1) {}

// Gauge class constructor
GaugeV1::GaugeV1(const Theme& theme, 
             const Dimensions& dimensions, 
             const Config& config) 
    : theme(theme), dims(dimensions), config(config), value(0.0f) {}

// Set gauge value
void GaugeV1::setValue(float newValue) {
    value = clamp(newValue, 0.0f, 100.0f);
}

// Set gauge scale
void GaugeV1::setScale(float scale) {
    dims.scaleRatio = scale;
}

// Set gauge base size
void GaugeV1::setBaseSize(float size) {
    dims.baseSize = size;
}

// Set gauge arc thickness
void GaugeV1::setArcThickness(float thickness) {
    dims.arcThickness = thickness;
}

// calculate gauge size
float GaugeV1::calculateGaugeSize() const {
    if (config.autoScale) {
        float screenSize = fmin(GetScreenWidth(), GetScreenHeight());
        return screenSize * config.screenSizeRatio * dims.scaleRatio;
    }
    return dims.baseSize * dims.scaleRatio;
}

// Draw gauge
void GaugeV1::draw(Vector2 center, const std::string& label) const {
    float gaugeSize = calculateGaugeSize();
    gaugeSize = clamp(gaugeSize, dims.minSize, dims.maxSize);

    float outerRadius = gaugeSize / 2;
    float innerRadius = outerRadius * (1.0f - dims.arcThickness);
    float endAngle = config.startAngle + config.totalAngle;
    float loadAngle = config.startAngle + (value / 100.0f) * config.totalAngle;

    // Draw background arc
    DrawArc(center, innerRadius, outerRadius, config.startAngle, endAngle, theme.arcBackgroundColor);

    // Draw active arc if the value is greater than 0
    if (value > 0) {
        DrawArc(center, innerRadius, outerRadius, config.startAngle, loadAngle, theme.arcActiveColor);
    }

    // Draw value in the center of the gauge
    float valueFontSize = gaugeSize * dims.textSizeRatio * dims.scaleRatio;
    float labelFontSize = valueFontSize * 0.50; // Make label slightly smaller than value
    
    // Draw percentage value
    const char* valueText = TextFormat("%.0f%%", value);
    Vector2 valueTextSize = MeasureTextEx(GetFontDefault(), valueText, valueFontSize, 1);
    DrawText(valueText, 
             center.x - valueTextSize.x / 2, 
             center.y - valueTextSize.y / 2, 
             valueFontSize, 
             theme.textColor);

    // Draw label based on method
    if (!label.empty()) {
        Vector2 labelTextSize = MeasureTextEx(GetFontDefault(), label.c_str(), labelFontSize, 1);
        
        if (config.method == 1) {
            // Draw label above the value
            DrawText(label.c_str(), 
                    center.x - labelTextSize.x / 2, 
                    center.y + valueTextSize.y / 2 + innerRadius/16, 
                    labelFontSize, 
                    theme.textColor);
        }
        else if (config.method == 2) {
            // Draw label below the value
            DrawText(label.c_str(), 
                    center.x + innerRadius/16, 
                    center.y + (outerRadius - innerRadius)/2 + innerRadius - labelTextSize.y / 2.1, 
                    labelFontSize, 
                    theme.textColor);
        }
    }
}