#ifndef TILEV1_H
#define TILEV1_H

#include "raylib.h"
#include "ThemeV1.h"
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <cmath>

struct TileConfig {
    int col;
    int row;
    int spanCols;
    int spanRows;
    int minSpanCols;
    int minSpanRows;
    std::string title;

    TileConfig shrunk(int newSpanCols, int newSpanRows) const {
        TileConfig c = *this;
        c.spanCols = newSpanCols;
        c.spanRows = newSpanRows;
        return c;
    }
};

struct LayoutConfig {
    static constexpr int gridCols = 4;
    static constexpr int gridRows = 4;
    static constexpr float statusBarH = 30.0f;
    static constexpr float tilePadding = 10.0f;
    static constexpr int maxTiles = 5;
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

    struct DisplacedPreview {
        int tileIndex;
        Rectangle originalBounds;
        Rectangle targetBounds;
    };
    std::vector<DisplacedPreview> displacedPreviews;

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

        Color dispFill = {theme.textMuted.r, theme.textMuted.g, theme.textMuted.b, 40};
        for (const auto& dp : displacedPreviews) {
            DrawRectangleRounded(dp.targetBounds, 0.06f, 12, dispFill);
            DrawRectangleRoundedLinesEx(dp.targetBounds, 0.06f, 12, 1.0f, theme.textMuted);
        }
    }

    Rectangle titleBar() const {
        float barH = bounds.height * 0.12f;
        if (barH < 26.0f) barH = 26.0f;
        if (barH > bounds.height * 0.45f) barH = bounds.height * 0.45f;
        return {bounds.x, bounds.y, bounds.width, barH};
    }

    Vector2 contentCenter() const {
        float tbH = std::max(bounds.height * 0.12f, 26.0f);
        if (tbH > bounds.height * 0.45f) tbH = bounds.height * 0.45f;
        return {bounds.x + bounds.width / 2, bounds.y + tbH + (bounds.height - tbH) / 2};
    }

    float contentHeight() const {
        float tbH = std::max(bounds.height * 0.12f, 26.0f);
        if (tbH > bounds.height * 0.45f) tbH = bounds.height * 0.45f;
        return bounds.height - tbH;
    }

    float contentWidth() const {
        return bounds.width * 0.88f;
    }

    // ─── Cell-based grid operations ───────────────────────────

    static constexpr int kMaxGrid = 8;

    static void buildOccupancy(const std::vector<TileV1>& tiles, int excludeIdx,
                               bool occupied[kMaxGrid][kMaxGrid], int gridRows, int gridCols) {
        for (int r = 0; r < gridRows; r++)
            for (int c = 0; c < gridCols; c++)
                occupied[r][c] = false;

        for (int i = 0; i < static_cast<int>(tiles.size()); i++) {
            if (i == excludeIdx) continue;
            const TileConfig& cfg = tiles[i].config;
            for (int r = cfg.row; r < cfg.row + cfg.spanRows && r < gridRows; r++)
                for (int c = cfg.col; c < cfg.col + cfg.spanCols && c < gridCols; c++)
                    occupied[r][c] = true;
        }
    }

    static bool fitsInGrid(const TileConfig& cfg, int targetRow, int targetCol,
                           bool occupied[kMaxGrid][kMaxGrid], int gridRows, int gridCols) {
        if (targetRow < 0 || targetCol < 0) return false;
        if (targetRow + cfg.spanRows > gridRows) return false;
        if (targetCol + cfg.spanCols > gridCols) return false;
        for (int r = targetRow; r < targetRow + cfg.spanRows; r++)
            for (int c = targetCol; c < targetCol + cfg.spanCols; c++)
                if (occupied[r][c]) return false;
        return true;
    }

    static void markCells(const TileConfig& cfg, bool occupied[kMaxGrid][kMaxGrid], bool value) {
        for (int r = cfg.row; r < cfg.row + cfg.spanRows && r < kMaxGrid; r++)
            for (int c = cfg.col; c < cfg.col + cfg.spanCols && c < kMaxGrid; c++)
                occupied[r][c] = value;
    }

    static int contiguousFreeRight(bool occupied[kMaxGrid][kMaxGrid], int row, int startCol, int gridCols) {
        int count = 0;
        for (int c = startCol; c < gridCols && !occupied[row][c]; c++) count++;
        return count;
    }

    static int contiguousFreeDown(bool occupied[kMaxGrid][kMaxGrid], int col, int startRow, int gridRows) {
        int count = 0;
        for (int r = startRow; r < gridRows && !occupied[r][col]; r++) count++;
        return count;
    }

    static int contiguousFreeLeft(bool occupied[kMaxGrid][kMaxGrid], int row, int startCol) {
        int count = 0;
        for (int c = startCol; c >= 0 && !occupied[row][c]; c--) count++;
        return count;
    }

    static int contiguousFreeUp(bool occupied[kMaxGrid][kMaxGrid], int col, int startRow) {
        int count = 0;
        for (int r = startRow; r >= 0 && !occupied[r][col]; r--) count++;
        return count;
    }

    static bool tryShiftTile(TileConfig& config, bool occupied[kMaxGrid][kMaxGrid],
                              int gridRows, int gridCols) {
        int bestDR = 0, bestDC = 0, bestSpanC = 0, bestSpanR = 0;
        int bestScore = -1;

        for (int trySC = config.spanCols; trySC >= config.minSpanCols; trySC--) {
            for (int trySR = config.spanRows; trySR >= config.minSpanRows; trySR--) {
                if (trySC == config.spanCols && trySR == config.spanRows) {
                    goto skip; // don't re-check current size (already tried by caller)
                }
                for (int tr = 0; tr < gridRows; tr++) {
                    for (int tc = 0; tc < gridCols; tc++) {
                        if (fitsInGrid(config.shrunk(trySC, trySR), tr, tc, occupied, gridRows, gridCols)) {
                            int score = (gridRows - abs(tr - config.row)) + (gridCols - abs(tc - config.col)) + trySC + trySR;
                            if (score > bestScore) {
                                bestScore = score;
                                bestDR = tr;
                                bestDC = tc;
                                bestSpanC = trySC;
                                bestSpanR = trySR;
                            }
                        }
                    }
                }
                skip:;
            }
        }

        // Try current span as well
        for (int tr = 0; tr < gridRows; tr++) {
            for (int tc = 0; tc < gridCols; tc++) {
                if (fitsInGrid(config, tr, tc, occupied, gridRows, gridCols)) {
                    int score = (gridRows - abs(tr - config.row)) + (gridCols - abs(tc - config.col)) + config.spanCols + config.spanRows;
                    if (score > bestScore) {
                        bestScore = score;
                        bestDR = tr;
                        bestDC = tc;
                        bestSpanC = config.spanCols;
                        bestSpanR = config.spanRows;
                    }
                }
            }
        }

        if (bestScore >= 0) {
            config.row = bestDR;
            config.col = bestDC;
            config.spanCols = bestSpanC;
            config.spanRows = bestSpanR;
            markCells(config, occupied, true);
            return true;
        }
        return false;
    }

    static void computePixelBounds(TileConfig& cfg, Rectangle& bounds,
                                    int screenW, int screenH, int gridCols, int gridRows, float padding) {
        float cellW = static_cast<float>(screenW) / gridCols;
        float cellH = static_cast<float>(screenH) / gridRows;
        bounds.x = cellW * cfg.col + padding;
        bounds.y = cellH * cfg.row + padding;
        bounds.width = cellW * cfg.spanCols - padding * 2;
        bounds.height = cellH * cfg.spanRows - padding * 2;
    }

    static bool anyPixelOverlaps(const std::vector<TileV1>& tiles) {
        for (int i = 0; i < static_cast<int>(tiles.size()); i++) {
            for (int j = i + 1; j < static_cast<int>(tiles.size()); j++) {
                if (CheckCollisionRecs(tiles[i].bounds, tiles[j].bounds))
                    return true;
            }
        }
        return false;
    }

    struct PlacementResult {
        bool accepted;
        int finalRow;
        int finalCol;
        int finalSpanCols;
        int finalSpanRows;
        std::vector<std::pair<int, TileConfig>> displacedTiles; // index, new config
    };

    static PlacementResult tryPlaceTile(const std::vector<TileV1>& tiles, int draggedIdx,
                                        int targetRow, int targetCol, int targetSpanCols, int targetSpanRows,
                                        int gridRows, int gridCols) {
        PlacementResult result = {false, 0, 0, 0, 0, {}};
        if (draggedIdx < 0 || draggedIdx >= static_cast<int>(tiles.size())) return result;

        TileConfig testCfg = tiles[draggedIdx].config;
        testCfg.col = targetCol;
        testCfg.row = targetRow;
        testCfg.spanCols = targetSpanCols;
        testCfg.spanRows = targetSpanRows;

        bool occupied[kMaxGrid][kMaxGrid];
        buildOccupancy(tiles, draggedIdx, occupied, gridRows, gridCols);

        // Step 1: does the dragged tile fit directly?
        if (fitsInGrid(testCfg, targetRow, targetCol, occupied, gridRows, gridCols)) {
            result.accepted = true;
            result.finalRow = targetRow;
            result.finalCol = targetCol;
            result.finalSpanCols = targetSpanCols;
            result.finalSpanRows = targetSpanRows;
            return result;
        }

        // Step 2: find conflicting tiles and try to displace them
        std::vector<int> conflicts;
        for (int i = 0; i < static_cast<int>(tiles.size()); i++) {
            if (i == draggedIdx) continue;
            const TileConfig& oc = tiles[i].config;
            bool overlaps = false;
            for (int r = targetRow; r < targetRow + targetSpanRows && r < gridRows; r++)
                for (int c = targetCol; c < targetCol + targetSpanCols && c < gridCols; c++)
                    if (occupied[r][c] && r >= oc.row && r < oc.row + oc.spanRows &&
                        c >= oc.col && c < oc.col + oc.spanCols)
                        { overlaps = true; break; }
            if (overlaps) {
                conflicts.push_back(i);
                // Unmark conflicting tile so shift can use that space
                markCells(tiles[i].config, occupied, false);
            }
        }

        if (conflicts.empty()) return result;

        bool allShifted = true;
        std::vector<TileConfig> newConfigs(conflicts.size());
        for (size_t ci = 0; ci < conflicts.size(); ci++) {
            newConfigs[ci] = tiles[conflicts[ci]].config;
        }

        // Try to place dragged tile first
        bool draggedPlaced = false;
        for (int tryR = targetRow; tryR < gridRows && !draggedPlaced; tryR++) {
            for (int tryC = 0; tryC < gridCols && !draggedPlaced; tryC++) {
                if (fitsInGrid(testCfg, tryR, tryC, occupied, gridRows, gridCols)) {
                    targetRow = tryR;
                    targetCol = tryC;
                    draggedPlaced = true;
                }
            }
        }

        if (draggedPlaced) {
            markCells(testCfg, occupied, true);
        } else {
            allShifted = false;
        }

        // Shift each conflicting tile
        if (allShifted) {
            for (size_t ci = 0; ci < conflicts.size(); ci++) {
                TileConfig& nc = newConfigs[ci];
                if (nc.spanCols < tiles[conflicts[ci]].config.minSpanCols) nc.spanCols = tiles[conflicts[ci]].config.minSpanCols;
                if (nc.spanRows < tiles[conflicts[ci]].config.minSpanRows) nc.spanRows = tiles[conflicts[ci]].config.minSpanRows;
                if (!tryShiftTile(nc, occupied, gridRows, gridCols)) {
                    allShifted = false;
                    break;
                }
            }
        }

        if (allShifted) {
            result.accepted = true;
            result.finalRow = targetRow;
            result.finalCol = targetCol;
            result.finalSpanCols = testCfg.spanCols;
            result.finalSpanRows = testCfg.spanRows;
            for (size_t ci = 0; ci < conflicts.size(); ci++)
                result.displacedTiles.push_back({conflicts[ci], newConfigs[ci]});
        }

        return result;
    }

    // ─── Drag and resize handling ─────────────────────────────

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
            updateGhostPreview(screenW, screenH, gridCols, gridRows, 10.0f, others);
            return;
        }
        if (beingDragged && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            bounds.x = mousePos.x - dragOffset.x;
            bounds.y = mousePos.y - dragOffset.y;
            if (bounds.x < 0) bounds.x = 0;
            if (bounds.y < 0) bounds.y = 0;
            if (bounds.x + bounds.width > screenW) bounds.x = screenW - bounds.width;
            if (bounds.y + bounds.height > screenH) bounds.y = screenH - bounds.height;
            updateGhostPreview(screenW, screenH, gridCols, gridRows, 10.0f, others);
            return;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (beingDragged || beingResized) {
                commitPlacement(screenW, screenH, gridCols, gridRows, 10.0f, others);
            }
            beingDragged = false;
            beingResized = false;
            hasSnapPreview = false;
            displacedPreviews.clear();
        }
    }

    void updateGhostPreview(int screenW, int screenH, int gridCols, int gridRows, float padding,
                            const std::vector<TileV1>& others) {
        float cellW = static_cast<float>(screenW) / gridCols;
        float cellH = static_cast<float>(screenH) / gridRows;

        int targetCol = std::max(0, std::min(gridCols - 1, static_cast<int>((bounds.x) / cellW + 0.5f)));
        int targetRow = std::max(0, std::min(gridRows - 1, static_cast<int>((bounds.y) / cellH + 0.5f)));
        int targetSpanCols = std::max(1, std::min(gridCols - targetCol, static_cast<int>(bounds.width / cellW + 0.5f)));
        int targetSpanRows = std::max(1, std::min(gridRows - targetRow, static_cast<int>(bounds.height / cellH + 0.5f)));

        snapPreview.x = cellW * targetCol + padding;
        snapPreview.y = cellH * targetRow + padding;
        snapPreview.width = cellW * targetSpanCols - padding * 2;
        snapPreview.height = cellH * targetSpanRows - padding * 2;
        hasSnapPreview = true;

        int myIndex = -1;
        for (int i = 0; i < static_cast<int>(others.size()); i++) {
            if (&others[i] == this) { myIndex = i; break; }
        }

        PlacementResult pr = tryPlaceTile(others, myIndex, targetRow, targetCol,
                                          targetSpanCols, targetSpanRows, gridRows, gridCols);

        displacedPreviews.clear();
        if (pr.accepted) {
            for (const auto& dp : pr.displacedTiles) {
                DisplacedPreview prev;
                prev.tileIndex = dp.first;
                prev.originalBounds = others[dp.first].bounds;
                Rectangle temp;
                TileConfig tc = dp.second;
                computePixelBounds(tc, temp, screenW, screenH, gridCols, gridRows, padding);
                prev.targetBounds = temp;
                displacedPreviews.push_back(prev);
            }
        }
    }

    void commitPlacement(int screenW, int screenH, int gridCols, int gridRows,
                         float padding, std::vector<TileV1>& others) {
        float cellW = static_cast<float>(screenW) / gridCols;
        float cellH = static_cast<float>(screenH) / gridRows;

        int targetCol = std::max(0, std::min(gridCols - 1, static_cast<int>((bounds.x) / cellW + 0.5f)));
        int targetRow = std::max(0, std::min(gridRows - 1, static_cast<int>((bounds.y) / cellH + 0.5f)));
        int targetSpanCols = std::max(1, std::min(gridCols - targetCol, static_cast<int>(bounds.width / cellW + 0.5f)));
        int targetSpanRows = std::max(1, std::min(gridRows - targetRow, static_cast<int>(bounds.height / cellH + 0.5f)));

        int myIndex = -1;
        for (int i = 0; i < static_cast<int>(others.size()); i++) {
            if (&others[i] == this) { myIndex = i; break; }
        }

        PlacementResult pr = tryPlaceTile(others, myIndex, targetRow, targetCol,
                                          targetSpanCols, targetSpanRows, gridRows, gridCols);

        if (pr.accepted) {
            int originalCol = config.col;
            int originalRow = config.row;
            int originalSpanCols = config.spanCols;
            int originalSpanRows = config.spanRows;
            std::unordered_map<int, TileConfig> displacedOriginals;
            for (const auto& dp : pr.displacedTiles) {
                displacedOriginals[dp.first] = others[dp.first].config;
            }

            config.col = pr.finalCol;
            config.row = pr.finalRow;
            config.spanCols = pr.finalSpanCols;
            config.spanRows = pr.finalSpanRows;
            bounds.x = cellW * pr.finalCol + padding;
            bounds.y = cellH * pr.finalRow + padding;
            bounds.width = cellW * pr.finalSpanCols - padding * 2;
            bounds.height = cellH * pr.finalSpanRows - padding * 2;

            for (const auto& dp : pr.displacedTiles) {
                TileV1& other = others[dp.first];
                other.config = dp.second;
                computePixelBounds(other.config, other.bounds, screenW, screenH, gridCols, gridRows, padding);
                other.lastScreenW = screenW;
                other.lastScreenH = screenH;
            }

            if (anyPixelOverlaps(others)) {
                // Hard invariant violated: revert to original
                config.col = originalCol;
                config.row = originalRow;
                config.spanCols = originalSpanCols;
                config.spanRows = originalSpanRows;
                bounds.x = cellW * originalCol + padding;
                bounds.y = cellH * originalRow + padding;
                bounds.width = cellW * originalSpanCols - padding * 2;
                bounds.height = cellH * originalSpanRows - padding * 2;

                for (const auto& dp : pr.displacedTiles) {
                    TileV1& other = others[dp.first];
                    other.config = displacedOriginals[dp.first];
                    computePixelBounds(other.config, other.bounds, screenW, screenH, gridCols, gridRows, padding);
                    other.lastScreenW = screenW;
                    other.lastScreenH = screenH;
                }
            }
        } else {
            // Rejected: snap back to original cell position
            bounds.x = cellW * config.col + padding;
            bounds.y = cellH * config.row + padding;
            bounds.width = cellW * config.spanCols - padding * 2;
            bounds.height = cellH * config.spanRows - padding * 2;
        }

        lastScreenW = screenW;
        lastScreenH = screenH;
        hasSnapPreview = false;
        displacedPreviews.clear();
    }

    static std::vector<TileV1> createDefaultTiles() {
        std::vector<TileV1> tiles;
        {
            TileConfig cfg = {0, 0, 2, 2, 1, 2, "CPU"};
            tiles.push_back(TileV1(cfg));
        }
        {
            TileConfig cfg = {0, 2, 2, 1, 1, 1, "RAM"};
            tiles.push_back(TileV1(cfg));
        }
        {
            TileConfig cfg = {2, 0, 2, 2, 1, 2, "GPU"};
            tiles.push_back(TileV1(cfg));
        }
        {
            TileConfig cfg = {2, 2, 2, 1, 1, 2, "Network"};
            tiles.push_back(TileV1(cfg));
        }
        {
            TileConfig cfg = {0, 3, 4, 1, 2, 1, "Storage"};
            tiles.push_back(TileV1(cfg));
        }
        return tiles;
    }
};

#endif
