#ifndef TILEV1_H
#define TILEV1_H

#include "raylib.h"
#include "ThemeV1.h"
#include <string>
#include <vector>

struct TileConfig {
    int col;
    int row;
    int spanCols;
    int spanRows;
    std::string title;
};

class TileV1 {
public:
    TileConfig config;
    Rectangle bounds;
    Rectangle snapPreview;
    bool beingDragged{false};
    bool beingResized{false};
    bool hasSnapPreview{false};
    Vector2 dragOffset{};
    float minWidth{120.0f};
    float minHeight{100.0f};
    int lastScreenW{0};
    int lastScreenH{0};

    TileV1() : config{}, bounds{}, snapPreview{} {}
    TileV1(const TileConfig& cfg) : config(cfg), bounds{}, snapPreview{} {}

    void computeBounds(int screenW, int screenH, int gridCols, int gridRows, float padding = 8.0f) {
        if (isDirty) return;
        if (bounds.width > 0.0f && bounds.height > 0.0f && screenW == lastScreenW && screenH == lastScreenH) return;
        lastScreenW = screenW;
        lastScreenH = screenH;
        float cellW = static_cast<float>(screenW) / gridCols;
        float cellH = static_cast<float>(screenH) / gridRows;
        bounds.x = cellW * config.col + padding;
        bounds.y = cellH * config.row + padding;
        bounds.width = cellW * config.spanCols - padding * 2;
        bounds.height = cellH * config.spanRows - padding * 2;
    }

    void drawFrame(const ThemeV1& theme, int titleFontSize) const {
        DrawRectangleRounded(bounds, 0.04f, 8, theme.tileBg);
        if (beingDragged) {
            DrawRectangleRoundedLines(bounds, 0.04f, 8, theme.barForeground);
        } else {
            DrawRectangleRoundedLines(bounds, 0.04f, 8, theme.tileBorder);
        }
        const char* titleText = config.title.c_str();
        int textW = MeasureText(titleText, titleFontSize);
        DrawText(titleText,
                 static_cast<int>(bounds.x + (bounds.width - textW) / 2),
                 static_cast<int>(bounds.y + 8),
                 titleFontSize, theme.titleText);
        DrawLine(static_cast<int>(bounds.x + bounds.width * 0.15f),
                 static_cast<int>(bounds.y + 8 + titleFontSize + 4),
                 static_cast<int>(bounds.x + bounds.width * 0.85f),
                 static_cast<int>(bounds.y + 8 + titleFontSize + 4),
                 theme.lineColor);

        float gs = 14.0f;
        Rectangle grip = {bounds.x + bounds.width - gs, bounds.y + bounds.height - gs, gs, gs};
        DrawTriangle(
            {grip.x + grip.width, grip.y},
            {grip.x + grip.width, grip.y + grip.height},
            {grip.x, grip.y + grip.height},
            theme.lineColor);
    }

    void drawSnapPreview(const ThemeV1& theme) const {
        if (!hasSnapPreview) return;
        DrawRectangleRounded(snapPreview, 0.04f, 8, {theme.barForeground.r, theme.barForeground.g, theme.barForeground.b, 60});
        DrawRectangleRoundedLines(snapPreview, 0.04f, 8, theme.barForeground);
    }

    Rectangle titleBar() const {
        float barH = bounds.height * 0.12f;
        if (barH < 24.0f) barH = 24.0f;
        return {bounds.x, bounds.y, bounds.width, barH};
    }

    void handleDrag(Vector2 mousePos, int screenW, int screenH, std::vector<TileV1>& others,
                    int gridCols, int gridRows) {
        if (!beingDragged && !beingResized) {
            Rectangle grip = {bounds.x + bounds.width - 14.0f, bounds.y + bounds.height - 14.0f, 14.0f, 14.0f};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, grip)) {
                beingResized = true;
                dragOffset = {mousePos.x - (bounds.x + bounds.width), mousePos.y - (bounds.y + bounds.height)};
                hasSnapPreview = false;
            }
            else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, titleBar())) {
                beingDragged = true;
                dragOffset = {mousePos.x - bounds.x, mousePos.y - bounds.y};
            }
        }

        if (beingResized && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float newW = mousePos.x - bounds.x - dragOffset.x;
            float newH = mousePos.y - bounds.y - dragOffset.y;
            if (newW > minWidth) bounds.width = newW;
            if (newH > minHeight) bounds.height = newH;
            return;
        }
        if (beingDragged && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            bounds.x = mousePos.x - dragOffset.x;
            bounds.y = mousePos.y - dragOffset.y;
            computeSnapPreview(screenW, screenH, gridCols, gridRows, 8.0f);
            return;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (beingDragged) {
                snapToGrid(screenW, screenH, gridCols, gridRows, 8.0f, others);
            }
            beingDragged = false;
            beingResized = false;
            hasSnapPreview = false;
        }
    }

    void computeSnapPreview(int screenW, int screenH, int gridCols, int gridRows, float padding) {
        float cellW = static_cast<float>(screenW) / gridCols;
        float cellH = static_cast<float>(screenH) / gridRows;
        float cx = bounds.x + bounds.width / 2;
        float cy = bounds.y + bounds.height / 2;
        int bestCol = std::max(0, std::min(gridCols - 1, static_cast<int>((cx - padding) / cellW)));
        int bestRow = std::max(0, std::min(gridRows - 1, static_cast<int>((cy - padding) / cellH)));
        snapPreview.x = cellW * bestCol + padding;
        snapPreview.y = cellH * bestRow + padding;
        snapPreview.width = cellW * config.spanCols - padding * 2;
        snapPreview.height = cellH * config.spanRows - padding * 2;
        hasSnapPreview = true;
    }

    void snapToGrid(int screenW, int screenH, int gridCols, int gridRows, float padding,
                    std::vector<TileV1>& others) {
        float cellW = static_cast<float>(screenW) / gridCols;
        float cellH = static_cast<float>(screenH) / gridRows;
        float cx = bounds.x + bounds.width / 2;
        float cy = bounds.y + bounds.height / 2;
        int bestCol = std::max(0, std::min(gridCols - 1, static_cast<int>((cx - padding) / cellW)));
        int bestRow = std::max(0, std::min(gridRows - 1, static_cast<int>((cy - padding) / cellH)));
        Rectangle target = {cellW * bestCol + padding, cellH * bestRow + padding,
                            cellW * config.spanCols - padding * 2, cellH * config.spanRows - padding * 2};

        // Collision avoidance: push overlapping tiles down
        for (auto& o : others) {
            if (&o == this) continue;
            if (CheckCollisionRecs(o.bounds, target)) {
                float shiftY = target.y + target.height + padding - o.bounds.y;
                o.bounds.y += shiftY;
                if (o.bounds.y + o.bounds.height > screenH - padding) {
                    o.bounds.y = padding;
                    o.bounds.x += o.bounds.width + padding;
                    if (o.bounds.x + o.bounds.width > screenW - padding) {
                        o.bounds.x = padding;
                    }
                }
            }
        }

        bounds = target;
        isDirty = true;
    }

    bool isDirty{false};

    Vector2 contentCenter() const {
        float titleBarH = std::max(bounds.height * 0.12f, 30.0f);
        return {bounds.x + bounds.width / 2, bounds.y + titleBarH + (bounds.height - titleBarH) / 2};
    }

    float contentHeight() const {
        float titleBarH = std::max(bounds.height * 0.12f, 30.0f);
        return bounds.height - titleBarH;
    }

    float contentWidth() const {
        return bounds.width * 0.9f;
    }
};

#endif
