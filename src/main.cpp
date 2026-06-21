#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include <windows.h>
#include "raylib.h"
#include "StatsCollector.h"
#include "BarWidget.h"
#include "GaugeWidget.h"
#include "AppTheme.h"
#include "ConfigManager.h"
#include "CsvLogger.h"
#include "TitleBar.h"
#include "GridLayout.h"
#include "StatsData.h"
#include "DiagnosticsOverlay.h"
#include "SettingsOverlay.h"
#include "LandingPage.h"
#include <cstdio>
#include <cstring>
#include <thread>
#include <string>
#include <vector>

StatsData statsData;
std::atomic<bool> running(true);
StatsCollector stats1;

AppState appState = LANDING;
AppTheme activeTheme = AppTheme::Dark();
int selectedThemeIndex = 0;
bool showDiagnostics = false;
bool showSettings = false;
bool tileEnabled[5] = {true, true, true, true, true};
bool loggingEnabled = false;
CsvLogger logger;

inline std::string formatValue(float value) {
    std::string s = std::to_string(value);
    size_t dot = s.find('.');
    return (dot != std::string::npos) ? s.substr(0, dot + 2) : s;
}

void updateStats() {
    while (running) {
        statsData.CPU_Freq.store(stats1.GETCPUFrequency());
        statsData.CPU_Util.store(stats1.GETCPUtilization());
        statsData.RAM_Util.store(stats1.GETRAMUtilization());
        statsData.Wifi_Send.store(stats1.GETWiFiSend());
        statsData.Wifi_Recv.store(stats1.GETWiFiReceive());
        statsData.Ether_Send.store(stats1.GETEthernetSend());
        statsData.Ether_Recv.store(stats1.GETEthernetReceive());
        int gpuCount = stats1.GETGPUCount();
        statsData.GPUCount.store(gpuCount);
        for (int i = 0; i < gpuCount && i < 4; i++) {
            statsData.GPU_Util[i].store(stats1.GETGPUUtilization(i));
        }

        int diskCount = stats1.GETDiskCount();
        statsData.DiskCount.store(diskCount);
        for (int i = 0; i < diskCount && i < 8; i++) {
            statsData.DiskUtil[i].store(stats1.GETDiskUtilization(i));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

extern "C" void InitTitleBarNative();

void renderLoop() {
    ConfigManager configManager;
    bool configLoaded = configManager.load();
    if (!configLoaded) {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER,
                          L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                          0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD appsUseLightTheme = 0;
            DWORD size = sizeof(appsUseLightTheme);
            if (RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr,
                                 reinterpret_cast<LPBYTE>(&appsUseLightTheme), &size) == ERROR_SUCCESS) {
                if (appsUseLightTheme) {
                    AppConfig ac = configManager.get();
                    ac.themeIndex = 1;
                    configManager.set(ac);
                }
            }
            RegCloseKey(hKey);
        }
    }
    const AppConfig& appCfg = configManager.get();

    int initW = appCfg.windowW > 0 ? appCfg.windowW : 1200;
    int initH = appCfg.windowH > 0 ? appCfg.windowH : 800;
    InitWindow(initW, initH, "Performance Monitor");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowState(FLAG_WINDOW_UNDECORATED);
    SetWindowMinSize(500, 400);
    SetTargetFPS(30);
    InitTitleBarNative();

    if (appCfg.themeIndex >= 0 && appCfg.themeIndex <= 2) {
        selectedThemeIndex = appCfg.themeIndex;
        const AppTheme themes[] = {AppTheme::Dark(), AppTheme::Light(), AppTheme::HighContrast()};
        activeTheme = themes[appCfg.themeIndex];
    }
    for (int i = 0; i < 5; i++) {
        tileEnabled[i] = appCfg.tileEnabled[i];
    }
    for (int i = 0; i < 8; i++) {
        stats1.SetDiskEnabled(i, appCfg.diskEnabled[i]);
    }
    for (int i = 0; i < 4; i++) {
        stats1.SetAdapterEnabled(i, appCfg.adapterEnabled[i]);
    }
    if (appCfg.windowX != -1 && appCfg.windowY != -1) {
        SetWindowPosition(appCfg.windowX, appCfg.windowY);
    }

    GaugeWidget::Theme gaugeTheme = GaugeWidget::Theme::fromAppTheme(activeTheme);
    GaugeWidget::Dimensions gaugeDims;
    GaugeWidget gaugeCPU(gaugeTheme, gaugeDims, GaugeWidget::Config::ConfigArc());
    GaugeWidget gaugeRAM(gaugeTheme, gaugeDims, GaugeWidget::Config::ConfigQuarter());

    BarWidget::Theme barTheme = BarWidget::Theme::fromAppTheme(activeTheme);
    BarWidget::Dimensions barDims;
    BarWidget::Config barCfg;
    std::vector<BarWidget> bars;
    for (int i = 0; i < 16; i++) bars.emplace_back(barTheme, barDims, barCfg);

    GridLayout grid;
    grid.offsetY = static_cast<float>(TITLE_BAR_H + 2 + 28);
    grid.tiles.push_back({0, 0, 2, 2, "CPU"});
    grid.tiles.push_back({2, 0, 2, 2, "RAM"});
    grid.tiles.push_back({0, 2, 2, 1, "GPU"});
    grid.tiles.push_back({2, 2, 2, 1, "Network"});
    grid.tiles.push_back({0, 3, 4, 1, "Storage"});

    int dragSourceIdx = -1;
    bool gridDragActive = false;

    bool windowShouldClose = false;
    while (!WindowShouldClose() && !windowShouldClose) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        float hScale = std::clamp(sh / 800.0f, 0.6f, 1.8f);
        int fontSize = static_cast<int>(std::max(18.0f * hScale, 10.0f));
        int titleSize = static_cast<int>(fontSize * 1.15f);

        if (appState == LANDING) {
            BeginDrawing();
            ClearBackground(activeTheme.landingBg);
            {
                TitleBar titleBar;
                if (titleBar.draw(sw, activeTheme, fontSize)) windowShouldClose = true;
            }
            DrawLandingPage(sw, sh, titleSize, fontSize, activeTheme, selectedThemeIndex, activeTheme, appState);
            EndDrawing();
            continue;
        }

        if (IsKeyPressed(KEY_F2)) showDiagnostics = !showDiagnostics;
        if (IsKeyPressed(KEY_F3)) showSettings = !showSettings;
        if (IsKeyPressed(KEY_F4)) {
            loggingEnabled = !loggingEnabled;
            if (loggingEnabled) {
                wchar_t cwd[MAX_PATH];
                GetCurrentDirectoryW(MAX_PATH, cwd);
                (void)logger.start(cwd);
            } else {
                logger.stop();
            }
        }

        gaugeTheme = GaugeWidget::Theme::fromAppTheme(activeTheme);
        barTheme = BarWidget::Theme::fromAppTheme(activeTheme);
        gaugeCPU.setTheme(gaugeTheme);
        gaugeRAM.setTheme(gaugeTheme);
        for (auto& b : bars) b.setTheme(barTheme);

        int tileAreaH = sh - 30 - static_cast<int>(grid.offsetY);

        Vector2 mousePos = GetMousePosition();

        GridLayout::PlacementResult placementPreview;
        bool hasPreview = false;

        if (!gridDragActive) {
            for (size_t i = 0; i < grid.tiles.size(); i++) {
                if (!tileEnabled[i]) continue;
                const auto& t = grid.tiles[i];
                Rectangle tb = t.titleBar();
                if (CheckCollisionPointRec(mousePos, tb) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (!(mousePos.y > tb.y && mousePos.y < tb.y + tb.height &&
                          mousePos.x > tb.x + t.bounds.width - 24.0f)) {
                        dragSourceIdx = static_cast<int>(i);
                        gridDragActive = true;
                    }
                    break;
                }
            }
        }

        if (gridDragActive) {
            float cellW = static_cast<float>(sw) / GRID_COLS;
            float cellH = static_cast<float>(tileAreaH) / GRID_ROWS;
            int targetCol = static_cast<int>(mousePos.x / cellW);
            int targetRow = static_cast<int>((mousePos.y - grid.offsetY) / cellH);
            if (targetCol < 0) targetCol = 0;
            if (targetCol + grid.tiles[dragSourceIdx].spanCols > GRID_COLS)
                targetCol = GRID_COLS - grid.tiles[dragSourceIdx].spanCols;
            if (targetRow < 0) targetRow = 0;
            if (targetRow + grid.tiles[dragSourceIdx].spanRows > GRID_ROWS)
                targetRow = GRID_ROWS - grid.tiles[dragSourceIdx].spanRows;

            placementPreview = grid.computePlacement(dragSourceIdx, targetCol, targetRow);
            hasPreview = placementPreview.valid;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                if (hasPreview) {
                    grid.tryPlace(dragSourceIdx, targetCol, targetRow);
                }
                dragSourceIdx = -1;
                hasPreview = false;
                placementPreview = {};
                gridDragActive = false;
            }
        }

        grid.layout(sw, tileAreaH);

        StatsData local;
        local.CPU_Freq = statsData.CPU_Freq.load();
        local.CPU_Util = statsData.CPU_Util.load();
        local.RAM_Util = statsData.RAM_Util.load();
        local.Wifi_Send = statsData.Wifi_Send.load();
        local.Wifi_Recv = statsData.Wifi_Recv.load();
        local.Ether_Send = statsData.Ether_Send.load();
        local.Ether_Recv = statsData.Ether_Recv.load();
        local.GPUCount = statsData.GPUCount.load();
        for (int i = 0; i < local.GPUCount && i < 4; i++) {
            local.GPU_Util[i] = statsData.GPU_Util[i].load();
        }
        local.DiskCount = statsData.DiskCount.load();
        for (int i = 0; i < local.DiskCount && i < 8; i++) {
            local.DiskUtil[i] = statsData.DiskUtil[i].load();
        }

        if (loggingEnabled && logger.isLogging()) {
            float ramUsed = stats1.GETRAMUsed();
            logger.writeRow(local.CPU_Freq, local.CPU_Util, ramUsed, local.RAM_Util,
                           local.GPU_Util[0], local.Wifi_Send, local.Wifi_Recv,
                           local.Ether_Send, local.Ether_Recv);
        }

        gaugeCPU.setValue(local.CPU_Util);
        gaugeRAM.setValue(local.RAM_Util);
        for (int i = 0; i < 16; i++) bars[i].setValue(0.0f);
        bars[0].setValue(local.CPU_Freq);
        bars[1].setValue(local.Wifi_Send);
        bars[2].setValue(local.Wifi_Recv);
        bars[3].setValue(local.Ether_Send);
        bars[4].setValue(local.Ether_Recv);
        bars[5].setValue(local.GPU_Util[0]);

        BeginDrawing();
        ClearBackground(activeTheme.windowBg);

        {
            TitleBar titleBar;
            if (titleBar.draw(sw, activeTheme, fontSize)) windowShouldClose = true;
        }

        for (size_t ti = 0; ti < grid.tiles.size(); ti++) {
            if (!tileEnabled[ti]) continue;
            const auto& tile = grid.tiles[ti];
            const Rectangle& b = tile.bounds;
            if (b.width < 20.0f) continue;

            constexpr float REF_W = 400.0f;
            constexpr float REF_H = 300.0f;
            float contentScale = std::min(b.width / REF_W, b.height / REF_H);
            if (contentScale < 0.55f) contentScale = 0.55f;
            if (contentScale > 2.0f) contentScale = 2.0f;

            float tileTitleSize = 18.0f * contentScale;
            if (tileTitleSize < 10.0f) tileTitleSize = 10.0f;
            if (tileTitleSize > 24.0f) tileTitleSize = 24.0f;

            Rectangle shadowRect = {b.x + 3, b.y + 3, b.width, b.height};
            DrawRectangleRounded(shadowRect, 0.06f, 12, activeTheme.tileShadow);
            DrawRectangleRounded(b, 0.06f, 12, activeTheme.tileBg);
            DrawRectangleRoundedLinesEx(b, 0.06f, 12, 1.0f, activeTheme.tileBorder);

            Rectangle tb = tile.titleBar();
            float clampedFont = tileTitleSize;
            float usableH = tb.height * 0.7f;
            if (clampedFont > usableH) clampedFont = usableH;
            if (clampedFont < 8.0f) clampedFont = 8.0f;
            float gripPad = 22.0f;
            float maxTextW = b.width - 24.0f - gripPad;
            if (maxTextW < 20.0f) maxTextW = 20.0f;

            int textW = MeasureText(tile.title.c_str(), static_cast<int>(clampedFont));
            if (textW > maxTextW) {
                while (clampedFont > 8.0f) {
                    clampedFont -= 1.0f;
                    textW = MeasureText(tile.title.c_str(), static_cast<int>(clampedFont));
                    if (textW <= maxTextW) break;
                }
            }
            int textX = static_cast<int>(b.x + (b.width - textW) / 2);
            if (textX < static_cast<int>(b.x + 4)) textX = static_cast<int>(b.x + 4);
            int textY = static_cast<int>(b.y + (tb.height - clampedFont) / 2);
            DrawText(tile.title.c_str(), textX, textY, static_cast<int>(clampedFont), activeTheme.titleText);

            float lineY = tb.y + tb.height;
            DrawLineEx({b.x + b.width * 0.12f, lineY}, {b.x + b.width * 0.88f, lineY}, 1.0f, activeTheme.lineColor);

            float contentTop = b.y + tb.height + 6.0f;
            float contentBot = b.y + b.height - 8.0f;
            float contentH = contentBot - contentTop;
            if (contentH < 10.0f) continue;
            float cw = b.width * 0.88f;
            float midX = b.x + b.width / 2;

            switch (ti) {
            case 0: {
                bool compact = (contentScale < 0.65f || contentH < 100.0f);
                const char* cpuModel = stats1.GETCPUModel();
                float modelFont = std::min(14.0f * contentScale, 16.0f);
                if (modelFont < 8.0f) modelFont = 8.0f;
                bool hasModel = cpuModel[0] != '\0';

                if (compact) {
                    float leftW = b.width * 0.48f;
                    float gaugeSize = std::min(180.0f * contentScale, contentH * 0.85f);
                    if (gaugeSize < 40.0f) gaugeSize = 40.0f;
                    if (gaugeSize > leftW * 0.95f) gaugeSize = leftW * 0.95f;
                    gaugeCPU.setAutoScale(false);
                    gaugeCPU.setBaseSize(gaugeSize);
                    float gaugeCenterY = contentTop + contentH * 0.50f;
                    gaugeCPU.draw({b.x + leftW * 0.50f, gaugeCenterY}, "");

                    float rightX = b.x + leftW + 8.0f;
                    float rightW = b.width - leftW - 16.0f;
                    float ry = contentTop + contentH * 0.25f;
                    if (hasModel) {
                        int mw = MeasureText(cpuModel, static_cast<int>(modelFont));
                        float mf = modelFont;
                        while (mf > 8.0f && mw > rightW) { mf -= 0.5f; mw = MeasureText(cpuModel, static_cast<int>(mf)); }
                        DrawText(cpuModel, static_cast<int>(rightX), static_cast<int>(ry), static_cast<int>(mf), activeTheme.textSecondary);
                        ry += mf + 4.0f;
                    }
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%.1f %%", local.CPU_Util.load());
                    int vw = MeasureText(buf, static_cast<int>(modelFont));
                    DrawText(buf, static_cast<int>(rightX + (rightW - vw) / 2), static_cast<int>(ry), static_cast<int>(modelFont), activeTheme.textPrimary);
                    ry += modelFont + 6.0f;
                    if (ry + 30.0f < contentBot) {
                        barDims.maxSize = rightW;
                        bars[0].setDimensions(barDims);
                        bars[0].draw({rightX + rightW / 2, ry + bars[0].getTotalHeight() * 0.45f}, "Freq", formatValue(local.CPU_Freq));
                    }
                } else {
                    float modelY = contentTop;
                    if (hasModel) {
                        int mw = MeasureText(cpuModel, static_cast<int>(modelFont));
                        float maxW = b.width - 16.0f;
                        float useFont = modelFont;
                        while (useFont > 7.0f && mw > maxW) { useFont -= 0.5f; mw = MeasureText(cpuModel, static_cast<int>(useFont)); }
                        if (mw > maxW) {
                            int line1Len = static_cast<int>(maxW * 0.7f);
                            std::string part1(cpuModel, line1Len);
                            DrawText(part1.c_str(), static_cast<int>(midX - MeasureText(part1.c_str(), static_cast<int>(useFont)) / 2),
                                     static_cast<int>(contentTop + 4.0f), static_cast<int>(useFont), activeTheme.textSecondary);
                            if (static_cast<int>(strlen(cpuModel)) > line1Len) {
                                static char buffer[256];
                                snprintf(buffer, sizeof(buffer), "%s", cpuModel + line1Len);
                                DrawText(buffer, static_cast<int>(midX - MeasureText(buffer, static_cast<int>(useFont)) / 2),
                                         static_cast<int>(contentTop + 4.0f + useFont + 2.0f), static_cast<int>(useFont), activeTheme.textSecondary);
                                modelY = contentTop + useFont * 2.0f + 6.0f;
                            } else {
                                modelY = contentTop + useFont + 6.0f;
                            }
                        } else {
                            DrawText(cpuModel, static_cast<int>(midX - mw / 2),
                                     static_cast<int>(contentTop + 4.0f), static_cast<int>(useFont), activeTheme.textSecondary);
                            modelY = contentTop + useFont + 6.0f;
                        }
                    }
                    float gaugeAvail = contentBot - modelY - 10.0f;
                    if (gaugeAvail > contentH * 0.55f) gaugeAvail = contentH * 0.55f;
                    float gaugeSize = std::min(180.0f * contentScale, gaugeAvail * 0.85f);
                    if (gaugeSize < 40.0f) gaugeSize = 40.0f;
                    gaugeCPU.setAutoScale(false);
                    gaugeCPU.setBaseSize(gaugeSize);
                    float gaugeCenterY = modelY + gaugeSize * 0.52f;
                    gaugeCPU.draw({midX, gaugeCenterY}, "Utilization");

                    float gaugeBottom = gaugeCenterY + gaugeSize * 0.42f;
                    float barAvail = contentBot - gaugeBottom - 4.0f;
                    if (barAvail > 30.0f) {
                        barDims.maxSize = cw * 0.88f;
                        bars[0].setDimensions(barDims);
                        float barTotalH = bars[0].getTotalHeight();
                        float barCenterY = gaugeBottom + std::max(barAvail * 0.55f, barTotalH * 0.55f);
                        if (barCenterY + barTotalH * 0.5f > contentBot) barCenterY = contentBot - barTotalH * 0.5f - 2.0f;
                        bars[0].draw({midX, barCenterY}, "Frequency", formatValue(local.CPU_Freq));
                    }
                }
                break;
            }
            case 1: {
                bool compact = (contentScale < 0.65f || contentH < 100.0f);
                if (compact) {
                    float leftW = b.width * 0.48f;
                    float gaugeSize = std::min(200.0f * contentScale, contentH * 0.85f);
                    if (gaugeSize < 40.0f) gaugeSize = 40.0f;
                    if (gaugeSize > leftW * 0.95f) gaugeSize = leftW * 0.95f;
                    gaugeRAM.setAutoScale(false);
                    gaugeRAM.setBaseSize(gaugeSize);
                    float gaugeCenterY = contentTop + contentH * 0.50f;
                    gaugeRAM.draw({b.x + leftW * 0.50f, gaugeCenterY}, "");

                    float rightX = b.x + leftW + 8.0f;
                    float rightW = b.width - leftW - 16.0f;
                    float fontS = std::min(14.0f * contentScale, 16.0f);
                    if (fontS < 10.0f) fontS = 10.0f;
                    const char* label = "Memory Load";
                    int lw = MeasureText(label, static_cast<int>(fontS));
                    DrawText(label, static_cast<int>(rightX + (rightW - lw) / 2), static_cast<int>(contentTop + contentH * 0.30f),
                             static_cast<int>(fontS), activeTheme.textSecondary);
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%.1f %%", local.RAM_Util.load());
                    int vw = MeasureText(buf, static_cast<int>(fontS + 2));
                    DrawText(buf, static_cast<int>(rightX + (rightW - vw) / 2), static_cast<int>(contentTop + contentH * 0.55f),
                             static_cast<int>(fontS + 2), activeTheme.textPrimary);
                } else {
                    float gaugeAvail = contentH * 0.70f;
                    float gaugeSize = std::min(200.0f * contentScale, gaugeAvail * 0.72f);
                    if (gaugeSize < 40.0f) gaugeSize = 40.0f;
                    gaugeRAM.setAutoScale(false);
                    gaugeRAM.setBaseSize(gaugeSize);
                    float gaugeCenterY = contentTop + contentH * 0.42f;
                    gaugeRAM.draw({midX, gaugeCenterY}, "Load");
                }
                break;
            }
            case 2: {
                const char* gpuModel = stats1.GETGPUModel();
                float modelFont = std::min(14.0f * contentScale, 16.0f);
                if (modelFont < 8.0f) modelFont = 8.0f;
                bool hasModel = gpuModel[0] != '\0';
                if (hasModel) {
                    int mw = MeasureText(gpuModel, static_cast<int>(modelFont));
                    float maxW = b.width - 16.0f;
                    float useFont = modelFont;
                    while (useFont > 7.0f && mw > maxW) { useFont -= 0.5f; mw = MeasureText(gpuModel, static_cast<int>(useFont)); }
                    DrawText(gpuModel, static_cast<int>(midX - mw / 2),
                             static_cast<int>(contentTop + 4.0f), static_cast<int>(useFont), activeTheme.textSecondary);
                }
                int gpuCount = local.GPUCount;
                if (gpuCount < 1) gpuCount = 1;
                float barsTop = contentTop + (hasModel ? modelFont + 10.0f : 4.0f);
                float availH = contentBot - barsTop - 6.0f;
                barDims.maxSize = cw * 0.78f;
                bars[5].setDimensions(barDims);
                float barTotalH = bars[5].getTotalHeight();
                float spacing = availH / static_cast<float>(gpuCount);
                if (spacing < barTotalH + 4.0f) spacing = barTotalH + 4.0f;
                if (spacing > 60.0f) spacing = 60.0f;
                float startY = barsTop + barTotalH * 0.55f;
                if (startY + (gpuCount - 1) * spacing + barTotalH * 0.55f > contentBot - 2.0f) {
                    spacing = (contentBot - 2.0f - startY - barTotalH * 0.55f) / std::max(1, gpuCount - 1);
                    if (spacing < barTotalH + 2.0f) spacing = barTotalH + 2.0f;
                }
                for (int g = 0; g < gpuCount && g < 4; g++) {
                    char gpuLabel[64];
                    const char* displayName = stats1.GETGPUDisplayName(g);
                    if (displayName[0] != '\0') {
                        snprintf(gpuLabel, sizeof(gpuLabel), "%s %%", displayName);
                    } else if (gpuCount == 1) {
                        snprintf(gpuLabel, sizeof(gpuLabel), "Utilization %%");
                    } else {
                        snprintf(gpuLabel, sizeof(gpuLabel), "GPU %d %%", g + 1);
                    }
                    bars[5 + g].setDimensions(barDims);
                    bars[5 + g].setValue(local.GPU_Util[g]);
                    bars[5 + g].draw({midX, startY + g * spacing}, gpuLabel, formatValue(local.GPU_Util[g]));
                }
                break;
            }
            case 3: {
                barDims.maxSize = cw * 0.85f;
                bars[1].setDimensions(barDims);
                float barTotalH = bars[1].getTotalHeight();
                float minSpacing = barTotalH + 6.0f;
                float desiredSpacing = std::max(minSpacing, contentH * 0.22f);
                if (desiredSpacing > 55.0f) desiredSpacing = 55.0f;
                float totalNeeded = desiredSpacing * 3.0f + barTotalH;
                float sp = desiredSpacing;
                if (totalNeeded > contentH) {
                    sp = (contentH - barTotalH) / 3.0f;
                    if (sp < minSpacing) sp = minSpacing;
                }
                float startY = contentTop + barTotalH * 0.55f;
                bars[1].draw({midX, startY + sp * 0.0f}, "WiFi Up", formatValue(local.Wifi_Send));
                bars[2].draw({midX, startY + sp * 1.0f}, "WiFi Down", formatValue(local.Wifi_Recv));
                bars[3].draw({midX, startY + sp * 2.0f}, "Eth Up", formatValue(local.Ether_Send));
                bars[4].draw({midX, startY + sp * 3.0f}, "Eth Down", formatValue(local.Ether_Recv));
                break;
            }
            case 4: {
                const auto& diskList = stats1.GetDisks();
                int diskCount = static_cast<int>(diskList.size());
                int enabledCount = 0;
                for (int i = 0; i < diskCount; i++) {
                    if (diskList[i].enabled) enabledCount++;
                }
                if (enabledCount == 0) {
                    DrawText("No drives enabled", static_cast<int>(midX - MeasureText("No drives enabled", fontSize) / 2),
                             static_cast<int>(contentTop + contentH * 0.45f), fontSize, activeTheme.textSecondary);
                } else {
                    int barIdx = 6;
                    barDims.maxSize = cw * 0.78f;
                    bars[6].setDimensions(barDims);
                    float barTotalH = bars[6].getTotalHeight();
                    float availH = contentH * 0.88f;
                    float barH = availH / static_cast<float>(enabledCount);
                    if (barH < barTotalH + 3.0f) barH = barTotalH + 3.0f;
                    if (barH > 45.0f) barH = 45.0f;
                    float startY = contentTop + barTotalH * 0.55f;
                    if (startY + (enabledCount - 1) * barH + barTotalH * 0.55f > contentBot - 4.0f) {
                        barH = (contentBot - 4.0f - startY - barTotalH * 0.55f) / std::max(1, enabledCount - 1);
                        if (barH < barTotalH + 2.0f) barH = barTotalH + 2.0f;
                    }
                    for (int d = 0; d < diskCount && barIdx < 14; d++) {
                        if (!diskList[d].enabled) continue;
                        char buf[64];
                        WideCharToMultiByte(CP_UTF8, 0, diskList[d].name.c_str(), -1, buf, sizeof(buf), nullptr, nullptr);
                        bars[barIdx].setValue(local.DiskUtil[d]);
                        bars[barIdx].setDimensions(barDims);
                        bars[barIdx].draw({midX, startY + (barIdx - 6) * barH}, buf, formatValue(local.DiskUtil[d]));
                        barIdx++;
                    }
                }
                break;
            }
            }
        }

        Vector2 mousePos2 = GetMousePosition();
        for (auto& b : bars) {
            if (b.isLabelTruncated() && CheckCollisionPointRec(mousePos2, b.getLastBarRect())) {
                Rectangle tr = b.getLastBarRect();
                const char* tipText = b.getLastLabel().c_str();
                int tipW = MeasureText(tipText, fontSize) + 16;
                int tipH = fontSize + 10;
                float tipX = tr.x + tr.width / 2 - tipW / 2;
                float tipY = tr.y - tipH - 4;
                if (tipY < 4.0f) tipY = tr.y + tr.height + 4;
                if (tipX < 2.0f) tipX = 2.0f;
                if (tipX + tipW > static_cast<float>(sw)) tipX = static_cast<float>(sw) - tipW - 2.0f;
                DrawRectangleRounded({tipX, tipY, static_cast<float>(tipW), static_cast<float>(tipH)}, 0.3f, 8, activeTheme.panelOverlay);
                DrawRectangleRoundedLinesEx({tipX, tipY, static_cast<float>(tipW), static_cast<float>(tipH)}, 0.3f, 8, 1.0f, activeTheme.accentColor);
                DrawText(tipText, static_cast<int>(tipX) + 8, static_cast<int>(tipY) + 5, fontSize, activeTheme.textPrimary);
            }
        }

        if (hasPreview) {
            for (size_t i = 0; i < placementPreview.affectedIndices.size(); i++) {
                int idx = placementPreview.affectedIndices[i];
                int col = placementPreview.newPositions[i].first;
                int row = placementPreview.newPositions[i].second;
                const auto& t = grid.tiles[idx];
                float cellW = static_cast<float>(sw) / GRID_COLS;
                float cellH = static_cast<float>(tileAreaH) / GRID_ROWS;
                Rectangle ghost = {
                    col * cellW,
                    row * cellH + grid.offsetY,
                    t.spanCols * cellW,
                    t.spanRows * cellH
                };
                Color glow = {activeTheme.accentColor.r, activeTheme.accentColor.g, activeTheme.accentColor.b, 80};
                DrawRectangleRounded(ghost, 0.06f, 12, glow);
                DrawRectangleRoundedLinesEx(ghost, 0.06f, 12, 2.0f, activeTheme.accentColor);
                const char* label = (idx == dragSourceIdx) ? t.title.c_str() : nullptr;
                if (label) {
                    float fs = std::min(ghost.width * 0.08f, ghost.height * 0.12f);
                    if (fs < 8.0f) fs = 8.0f;
                    if (fs > 18.0f) fs = 18.0f;
                    int tw = MeasureText(label, static_cast<int>(fs));
                    DrawText(label, static_cast<int>(ghost.x + (ghost.width - tw) / 2),
                             static_cast<int>(ghost.y + 4.0f), static_cast<int>(fs), activeTheme.accentColor);
                }
            }
        }

        DrawFPS(sw - 80, 12);

        {
            int barH = 30;
            Rectangle statusBg = {0, static_cast<float>(sh - barH), static_cast<float>(sw), static_cast<float>(barH)};
            DrawRectangleRec(statusBg, activeTheme.windowBg);
            DrawLineEx({0, static_cast<float>(sh - barH)}, {static_cast<float>(sw), static_cast<float>(sh - barH)}, 1.0f, activeTheme.lineColor);

            int statusFont = std::max(fontSize - 4, 9);
            const char* logStatus = loggingEnabled ? "F4:Log [ON]" : "F4:Log [OFF]";
            Color logCol = loggingEnabled ? activeTheme.accentColor : activeTheme.textMuted;
            int logW = MeasureText(logStatus, statusFont);
            DrawText(logStatus, sw - logW - 12, sh - barH + 6, statusFont, logCol);
            DrawText("F2:Diag  F3:Settings  F4:Log", 12, sh - barH + 6, statusFont, activeTheme.textMuted);
        }

        if (showDiagnostics) DrawDiagnosticsOverlay(sw, sh, fontSize, activeTheme, statsData, stats1);
        if (showSettings) DrawSettingsOverlay(sw, sh, fontSize, activeTheme, tileEnabled, stats1);

        EndDrawing();
    }

    running = false;

    AppConfig saveCfg;
    saveCfg.themeIndex = selectedThemeIndex;
    for (int i = 0; i < 5; i++) saveCfg.tileEnabled[i] = tileEnabled[i];
    saveCfg.windowX = GetWindowPosition().x;
    saveCfg.windowY = GetWindowPosition().y;
    saveCfg.windowW = GetScreenWidth();
    saveCfg.windowH = GetScreenHeight();
    {
        const auto& disks = stats1.GetDisks();
        for (int i = 0; i < 8; i++) {
            saveCfg.diskEnabled[i] = (i < static_cast<int>(disks.size())) ? disks[i].enabled : true;
        }
    }
    {
        const auto& adapters = stats1.GetAdapters();
        for (int i = 0; i < 4; i++) {
            saveCfg.adapterEnabled[i] = (i < static_cast<int>(adapters.size())) ? adapters[i].enabled : true;
        }
    }
    (void)configManager.save(saveCfg);
    logger.stop();

    CloseWindow();
}

int main() {
    std::thread dataThread(updateStats);
    std::thread renderThread(renderLoop);
    dataThread.join();
    renderThread.join();
    return 0;
}
