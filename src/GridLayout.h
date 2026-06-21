#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include "raylib.h"
#include <string>
#include <vector>
#include <utility>
#include <cstring>

constexpr int GRID_COLS = 4;
constexpr int GRID_ROWS = 4;

struct TileConstraints {
    int minCols{1};
    int minRows{1};
    int maxCols{GRID_COLS};
    int maxRows{GRID_ROWS};
};

inline TileConstraints GetTileConstraints(const std::string& title) {
    if (title == "CPU") return {2, 2, 4, 4};
    if (title == "RAM") return {2, 2, 4, 4};
    if (title == "GPU") return {2, 1, 4, 3};
    if (title == "Network") return {2, 1, 4, 3};
    if (title == "Storage") return {2, 1, 4, 3};
    return {1, 1, 4, 4};
}

struct OccupancyGrid {
    int owner[GRID_ROWS][GRID_COLS]{};

    OccupancyGrid() {
        for (int r = 0; r < GRID_ROWS; r++)
            for (int c = 0; c < GRID_COLS; c++)
                owner[r][c] = -1;
    }

    void clear() {
        for (int r = 0; r < GRID_ROWS; r++)
            for (int c = 0; c < GRID_COLS; c++)
                owner[r][c] = -1;
    }

    bool isFree(int col, int row, int spanC, int spanR, int excludeIdx = -1) const {
        if (col < 0 || row < 0 || col + spanC > GRID_COLS || row + spanR > GRID_ROWS)
            return false;
        for (int r = row; r < row + spanR; r++)
            for (int c = col; c < col + spanC; c++)
                if (owner[r][c] >= 0 && owner[r][c] != excludeIdx)
                    return false;
        return true;
    }

    int countFreeInDirection(int fromCol, int fromRow, int spanC, int spanR,
                             int dirCol, int dirRow) const {
        int count = 0;
        int c = fromCol + dirCol * spanC;
        int r = fromRow + dirRow * spanR;
        while (c >= 0 && r >= 0 && c + spanC <= GRID_COLS && r + spanR <= GRID_ROWS) {
            bool allFree = true;
            for (int rr = r; rr < r + spanR && allFree; rr++)
                for (int cc = c; cc < c + spanC && allFree; cc++)
                    if (owner[rr][cc] >= 0) allFree = false;
            if (!allFree) break;
            count++;
            c += dirCol;
            r += dirRow;
        }
        return count;
    }
};

struct TileItem {
    int col{0}, row{0};
    int spanCols{1}, spanRows{1};
    std::string title;
    Rectangle bounds{};

    void computeBounds(int screenW, int screenH, float offsetY) {
        float cellW = static_cast<float>(screenW) / GRID_COLS;
        float cellH = static_cast<float>(screenH) / GRID_ROWS;
        bounds.x = col * cellW;
        bounds.y = row * cellH + offsetY;
        bounds.width = spanCols * cellW;
        bounds.height = spanRows * cellH;
    }

    Rectangle titleBar() const {
        float h = bounds.height * 0.12f;
        if (h < 22.0f) h = 22.0f;
        if (h > bounds.height * 0.45f) h = bounds.height * 0.45f;
        return {bounds.x, bounds.y, bounds.width, h};
    }

    float contentWidth() const { return bounds.width * 0.88f; }
};

struct GridLayout {
    std::vector<TileItem> tiles;
    float offsetY{32.0f};

    OccupancyGrid buildOccupancy(int excludeIdx = -1) const {
        OccupancyGrid g;
        for (size_t i = 0; i < tiles.size(); i++) {
            if (static_cast<int>(i) == excludeIdx) continue;
            const auto& t = tiles[i];
            for (int r = t.row; r < t.row + t.spanRows; r++)
                for (int c = t.col; c < t.col + t.spanCols; c++)
                    if (r < GRID_ROWS && c < GRID_COLS) g.owner[r][c] = static_cast<int>(i);
        }
        return g;
    }

    bool validateNoOverlap() const {
        OccupancyGrid g;
        for (size_t i = 0; i < tiles.size(); i++) {
            const auto& t = tiles[i];
            for (int r = t.row; r < t.row + t.spanRows; r++) {
                for (int c = t.col; c < t.col + t.spanCols; c++) {
                    if (r < 0 || r >= GRID_ROWS || c < 0 || c >= GRID_COLS) return false;
                    if (g.owner[r][c] >= 0) return false;
                    g.owner[r][c] = static_cast<int>(i);
                }
            }
        }
        return true;
    }

    void layout(int screenW, int screenH) {
        for (auto& t : tiles)
            t.computeBounds(screenW, screenH, offsetY);
    }

    int tileAtPos(float px, float py, int screenW, int screenH) const {
        float cellW = static_cast<float>(screenW) / GRID_COLS;
        float cellH = static_cast<float>(screenH) / GRID_ROWS;
        int col = static_cast<int>(px / cellW);
        int row = static_cast<int>((py - offsetY) / cellH);
        if (col < 0) col = 0; if (col >= GRID_COLS) col = GRID_COLS - 1;
        if (row < 0) row = 0; if (row >= GRID_ROWS) row = GRID_ROWS - 1;
        OccupancyGrid g = buildOccupancy();
        if (g.owner[row][col] >= 0) return g.owner[row][col];
        return -1;
    }

    struct PlacementResult {
        bool valid{false};
        std::vector<int> affectedIndices;
        std::vector<std::pair<int,int>> newPositions;  // (col, row) per affected
    };

    PlacementResult computePlacement(int dragIdx, int targetCol, int targetRow) const {
        PlacementResult result;
        if (dragIdx < 0 || dragIdx >= static_cast<int>(tiles.size())) return result;

        const auto& dragTile = tiles[dragIdx];
        int spanC = dragTile.spanCols;
        int spanR = dragTile.spanRows;

        if (targetCol + spanC > GRID_COLS) targetCol = GRID_COLS - spanC;
        if (targetRow + spanR > GRID_ROWS) targetRow = GRID_ROWS - spanR;
        if (targetCol < 0) targetCol = 0;
        if (targetRow < 0) targetRow = 0;

        OccupancyGrid g = buildOccupancy(dragIdx);

        if (g.isFree(targetCol, targetRow, spanC, spanR)) {
            result.valid = true;
            result.affectedIndices.push_back(dragIdx);
            result.newPositions.push_back({targetCol, targetRow});
            return result;
        }

        std::vector<int> conflicting;
        for (int r = targetRow; r < targetRow + spanR; r++)
            for (int c = targetCol; c < targetCol + spanC; c++)
                if (g.owner[r][c] >= 0) {
                    bool found = false;
                    for (int ci : conflicting)
                        if (ci == g.owner[r][c]) { found = true; break; }
                    if (!found) conflicting.push_back(g.owner[r][c]);
                }

        if (conflicting.empty()) {
            result.valid = false;
            return result;
        }

        static const int dirs[4][2] = {{0,-1}, {0,1}, {-1,0}, {1,0}};
        int bestDir = -1;
        int bestFree = -1;
        for (int d = 0; d < 4; d++) {
            int totalFree = 0;
            for (int ci : conflicting) {
                const auto& ct = tiles[ci];
                int f = g.countFreeInDirection(ct.col, ct.row, ct.spanCols, ct.spanRows,
                                                dirs[d][0], dirs[d][1]);
                totalFree += f;
            }
            if (totalFree > bestFree) {
                bestFree = totalFree;
                bestDir = d;
            }
        }

        if (bestDir < 0) return result;

        std::vector<std::pair<int,int>> proposed;
        std::vector<int> toMove;
        for (int ci : conflicting) {
            toMove.push_back(ci);
            int newCol = tiles[ci].col + dirs[bestDir][0];
            int newRow = tiles[ci].row + dirs[bestDir][1];
            if (newCol < 0 || newRow < 0 ||
                newCol + tiles[ci].spanCols > GRID_COLS ||
                newRow + tiles[ci].spanRows > GRID_ROWS) {
                return result;
            }
            proposed.push_back({newCol, newRow});
        }

        OccupancyGrid g2 = buildOccupancy(dragIdx);
        for (int ci : conflicting) {
            for (int r = tiles[ci].row; r < tiles[ci].row + tiles[ci].spanRows; r++)
                for (int c = tiles[ci].col; c < tiles[ci].col + tiles[ci].spanCols; c++)
                    if (r < GRID_ROWS && c < GRID_COLS) g2.owner[r][c] = -1;
        }

        for (size_t i = 0; i < conflicting.size(); i++) {
            int ci = conflicting[i];
            int nc = proposed[i].first;
            int nr = proposed[i].second;
            if (!g2.isFree(nc, nr, tiles[ci].spanCols, tiles[ci].spanRows)) {
                return result;
            }
            for (int r = nr; r < nr + tiles[ci].spanRows; r++)
                for (int c = nc; c < nc + tiles[ci].spanCols; c++)
                    g2.owner[r][c] = ci;
        }

        if (!g2.isFree(targetCol, targetRow, spanC, spanR, dragIdx)) {
            return result;
        }

        result.valid = true;
        result.affectedIndices.push_back(dragIdx);
        result.newPositions.push_back({targetCol, targetRow});
        for (size_t i = 0; i < conflicting.size(); i++) {
            result.affectedIndices.push_back(conflicting[i]);
            result.newPositions.push_back(proposed[i]);
        }
        return result;
    }

    bool tryPlace(int dragIdx, int targetCol, int targetRow) {
        auto result = computePlacement(dragIdx, targetCol, targetRow);
        if (!result.valid) return false;

        for (size_t i = 0; i < result.affectedIndices.size(); i++) {
            int idx = result.affectedIndices[i];
            tiles[idx].col = result.newPositions[i].first;
            tiles[idx].row = result.newPositions[i].second;
        }
        return validateNoOverlap();
    }

    bool canFitAnywhere(int tileIdx) const {
        OccupancyGrid g = buildOccupancy(tileIdx);
        const auto& t = tiles[tileIdx];
        for (int r = 0; r <= GRID_ROWS - t.spanRows; r++)
            for (int c = 0; c <= GRID_COLS - t.spanCols; c++)
                if (g.isFree(c, r, t.spanCols, t.spanRows))
                    return true;
        return false;
    }
};

#endif
