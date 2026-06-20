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

    void computeBounds(int screenW, int screenH, int gridCols, int gridRows, float padding = 10.0f) {
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

    void drawFrame(const ThemeV1& theme, float tileFontSize) const {
        if (bounds.width < 20.0f) return;

        Rectangle shadowRect = {bounds.x + 3, bounds.y + 3, bounds.width, bounds.height};
        DrawRectangleRounded(shadowRect, 0.06f, 12, theme.tileShadow);

        DrawRectangleRounded(bounds, 0.06f, 12, theme.tileBg);

        Color borderCol = beingDragged ? theme.accentColor : theme.tileBorder;
        float borderThick = beingDragged ? 2.0f : 1.0f;
        DrawRectangleRoundedLinesEx(bounds, 0.06f, 12, borderThick, borderCol);

        Rectangle tb = titleBar();
        float clampedFont = tileFontSize;
        float usableH = tb.height * 0.7f;
        if (clampedFont > usableH) clampedFont = usableH;
        if (clampedFont < 8.0f) clampedFont = 8.0f;

        float gripPad = 22.0f;
        float maxTextW = bounds.width - 24.0f - gripPad;
        if (maxTextW < 20.0f) maxTextW = 20.0f;

        const char* titleText = config.title.c_str();
        int textW = MeasureText(titleText, static_cast<int>(clampedFont));
        if (textW > maxTextW) {
            while (clampedFont > 8.0f) {
                clampedFont -= 1.0f;
                textW = MeasureText(titleText, static_cast<int>(clampedFont));
                if (textW <= maxTextW) break;
            }
        }
        int textX = static_cast<int>(bounds.x + (bounds.width - textW) / 2);
        if (textX < static_cast<int>(bounds.x + 4)) textX = static_cast<int>(bounds.x + 4);
        int textY = static_cast<int>(bounds.y + (tb.height - clampedFont) / 2);

        DrawText(titleText, textX, textY, static_cast<int>(clampedFont), theme.titleText);

        float lineY = tb.y + tb.height;
        DrawLineEx({bounds.x + bounds.width * 0.12f, lineY},
                   {bounds.x + bounds.width * 0.88f, lineY},
                   1.0f, theme.lineColor);

        float gs = 12.0f;
        float pad = 4.0f;
        Rectangle grip = {bounds.x + bounds.width - gs - pad, bounds.y + bounds.height - gs - pad, gs, gs};
        DrawTriangleLines(
            {grip.x + grip.width, grip.y},
            {grip.x + grip.width, grip.y + grip.height},
            {grip.x, grip.y + grip.height},
            beingResized ? theme.accentColor : theme.textMuted);
    }

    void drawSnapPreview(const ThemeV1& theme) const {
        if (!hasSnapPreview) return;
        Color ghostFill = {theme.accentColor.r, theme.accentColor.g, theme.accentColor.b, 50};
        DrawRectangleRounded(snapPreview, 0.06f, 12, ghostFill);
        DrawRectangleRoundedLinesEx(snapPreview, 0.06f, 12, 1.5f, theme.accentColor);
    }

    Rectangle titleBar() const {
        float barH = bounds.height * 0.12f;
        if (barH < 26.0f) barH = 26.0f;
        if (barH > bounds.height * 0.45f) barH = bounds.height * 0.45f;
        return {bounds.x, bounds.y, bounds.width, barH};
    }

    void handleDrag(Vector2 mousePos, int screenW, int screenH, std::vector<TileV1>& others,
                    int gridCols, int gridRows) {
        if (!beingDragged && !beingResized) {
            Rectangle grip = {bounds.x + bounds.width - 16.0f, bounds.y + bounds.height - 16.0f, 16.0f, 16.0f};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, grip)) {
                beingResized = true;
                dragOffset = {mousePos.x - (bounds.x + bounds.width), mousePos.y - (bounds.y + bounds.height)};
                hasSnapPreview = false;
            }
            else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, titleBar())) {
                beingDragged = true;
                dragOffset = {mousePos.x - bounds.x, mousePos.y - bounds.y};
            }
            return;
        }

        if (beingResized && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float newW = mousePos.x - bounds.x - dragOffset.x;
            float newH = mousePos.y - bounds.y - dragOffset.y;
            if (newW > minWidth) bounds.width = newW;
            if (newH > minHeight) bounds.height = newH;
            computeSnapPreview(screenW, screenH, gridCols, gridRows, 10.0f);
            return;
        }
        if (beingDragged && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            bounds.x = mousePos.x - dragOffset.x;
            bounds.y = mousePos.y - dragOffset.y;
            if (bounds.x < 0) bounds.x = 0;
            if (bounds.y < 0) bounds.y = 0;
            if (bounds.x + bounds.width > screenW) bounds.x = screenW - bounds.width;
            if (bounds.y + bounds.height > screenH) bounds.y = screenH - bounds.height;
            computeSnapPreview(screenW, screenH, gridCols, gridRows, 10.0f);
            return;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (beingDragged || beingResized) {
                snapToGrid(screenW, screenH, gridCols, gridRows, 10.0f, others);
            }
            beingDragged = false;
            beingResized = false;
            hasSnapPreview = false;
        }
    }

    void computeSnapPreview(int screenW, int screenH, int gridCols, int gridRows, float padding) {
        float cellW = static_cast<float>(screenW) / gridCols;
        float cellH = static_cast<float>(screenH) / gridRows;
        float leftEdge = bounds.x;
        float topEdge = bounds.y;
        int bestCol = std::max(0, std::min(gridCols - 1, static_cast<int>((leftEdge) / cellW + 0.5f)));
        int bestRow = std::max(0, std::min(gridRows - 1, static_cast<int>((topEdge) / cellH + 0.5f)));
        int previewSpanCols = std::max(1, std::min(gridCols - bestCol, static_cast<int>(bounds.width / cellW + 0.5f)));
        int previewSpanRows = std::max(1, std::min(gridRows - bestRow, static_cast<int>(bounds.height / cellH + 0.5f)));
        snapPreview.x = cellW * bestCol + padding;
        snapPreview.y = cellH * bestRow + padding;
        snapPreview.width = cellW * previewSpanCols - padding * 2;
        snapPreview.height = cellH * previewSpanRows - padding * 2;
        hasSnapPreview = true;
    }

    void snapToGrid(int screenW, int screenH, int gridCols, int gridRows, float padding,
                    std::vector<TileV1>& others) {
        float cellW = static_cast<float>(screenW) / gridCols;
        float cellH = static_cast<float>(screenH) / gridRows;
        float leftEdge = bounds.x;
        float topEdge = bounds.y;
        int bestCol = std::max(0, std::min(gridCols - 1, static_cast<int>((leftEdge) / cellW + 0.5f)));
        int bestRow = std::max(0, std::min(gridRows - 1, static_cast<int>((topEdge) / cellH + 0.5f)));

        int bestSpanCols = std::max(1, std::min(gridCols - bestCol, static_cast<int>(bounds.width / cellW + 0.5f)));
        int bestSpanRows = std::max(1, std::min(gridRows - bestRow, static_cast<int>(bounds.height / cellH + 0.5f)));

        config.col = bestCol;
        config.row = bestRow;
        config.spanCols = bestSpanCols;
        config.spanRows = bestSpanRows;

        Rectangle target = {cellW * bestCol + padding, cellH * bestRow + padding,
                            cellW * bestSpanCols - padding * 2, cellH * bestSpanRows - padding * 2};

        resolveCollisions(others, target, gridCols, gridRows, cellW, cellH, padding);

        bounds = target;
    }

    void resolveCollisions(std::vector<TileV1>& others, const Rectangle& target,
                           int gridCols, int gridRows, float cellW, float cellH, float padding) {
        Rectangle occupied = target;
        for (auto& o : others) {
            if (&o == this) continue;
            if (!CheckCollisionRecs(o.bounds, occupied)) continue;

            bool placed = false;
            for (int tryRow = 0; tryRow < gridRows && !placed; tryRow++) {
                for (int tryCol = 0; tryCol < gridCols && !placed; tryCol++) {
                    Rectangle candidate = {cellW * tryCol + padding, cellH * tryRow + padding,
                                           cellW * o.config.spanCols - padding * 2,
                                           cellH * o.config.spanRows - padding * 2};
                    if (candidate.x + candidate.width > cellW * gridCols) continue;
                    if (candidate.y + candidate.height > cellH * gridRows) continue;

                    bool overlaps = false;
                    for (auto& other : others) {
                        if (&other == &o) continue;
                        if (&other == this) {
                            if (CheckCollisionRecs(candidate, target)) { overlaps = true; break; }
                        } else {
                            if (CheckCollisionRecs(candidate, other.bounds)) { overlaps = true; break; }
                        }
                    }
                    if (!overlaps) {
                        o.bounds = candidate;
                        o.config.col = tryCol;
                        o.config.row = tryRow;
                        placed = true;
                    }
                }
            }
            if (!placed) {
                o.bounds.y = occupied.y + occupied.height + padding;
                if (o.bounds.y + o.bounds.height > cellH * gridRows) {
                    o.bounds.y = padding;
                    o.bounds.x += o.bounds.width + padding;
                    if (o.bounds.x + o.bounds.width > cellW * gridCols) {
                        o.bounds.x = padding;
                    }
                }
                o.config.col = std::max(0, std::min(gridCols - 1, static_cast<int>((o.bounds.x + o.bounds.width / 2 - padding) / cellW)));
                o.config.row = std::max(0, std::min(gridRows - 1, static_cast<int>((o.bounds.y + o.bounds.height / 2 - padding) / cellH)));
            }
        }
    }

    Vector2 contentCenter() const {
        float titleBarH = std::max(bounds.height * 0.12f, 26.0f);
        if (titleBarH > bounds.height * 0.45f) titleBarH = bounds.height * 0.45f;
        return {bounds.x + bounds.width / 2, bounds.y + titleBarH + (bounds.height - titleBarH) / 2};
    }

    float contentHeight() const {
        float titleBarH = std::max(bounds.height * 0.12f, 26.0f);
        if (titleBarH > bounds.height * 0.45f) titleBarH = bounds.height * 0.45f;
        return bounds.height - titleBarH;
    }

    float contentWidth() const {
        return bounds.width * 0.88f;
    }
};

#endif
