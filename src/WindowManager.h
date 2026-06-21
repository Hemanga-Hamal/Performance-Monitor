#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include <windows.h>
#include "raylib.h"
#include "Stats.h"
#include "Bar.h"
#include "Gauge.h"
#include "Theme.h"
#include "Tile.h"
#include "Config.h"
#include "Logger.h"
#include "Rendering.h"
#include "LandingPage.h"
#include "Overlays.h"
#include <cstdio>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <algorithm>

class WindowManager {
public:
    static int run() {
        WindowManager wm;
        std::thread dataThread([&wm]() { wm.updateStats(); });
        wm.renderLoop();
        dataThread.join();
        return 0;
    }

private:
    StatsData statsData;
    std::atomic<bool> dataRunning{true};
    Stats stats;
    AppState appState{LANDING};
    Theme activeTheme{Theme::Dark()};
    int selectedThemeIndex{0};
    bool showDiagnostics{false};
    bool showSettings{false};
    bool tileEnabled[5]{true, true, true, true, true};
    bool loggingEnabled{false};
    Logger logger;

    void updateStats() {
        bool wroteModels = false;
        while (dataRunning) {
            statsData.CPU_Freq.store(stats.GETCPUFrequency());
            statsData.CPU_Util.store(stats.GETCPUtilization());
            statsData.RAM_Total.store(stats.GETRAMTotal());
            statsData.RAM_Used.store(stats.GETRAMUsed());
            statsData.RAM_Util.store(stats.GETRAMUtilization());
            statsData.Wifi_Send.store(stats.GETWiFiSend());
            statsData.Wifi_Recv.store(stats.GETWiFiReceive());
            statsData.Ether_Send.store(stats.GETEthernetSend());
            statsData.Ether_Recv.store(stats.GETEthernetReceive());
            if (!wroteModels) {
                statsData.cpuModel = stats.GETCPUModel();
                statsData.gpuModel = stats.GETGPUModel();
                wroteModels = true;
            }
            int gpuCount = stats.GETGPUCount();
            statsData.GPUCount.store(gpuCount);
            for (int i = 0; i < gpuCount && i < 4; i++) {
                statsData.GPU_Util[i].store(stats.GETGPUUtilization(i));
                statsData.GPU_VRAM[i].store(stats.GETGPUVRAMUsed(i));
                statsData.GPU_VRAMTotal[i].store(stats.GETGPUVRAMTotal(i));
                statsData.GPU_Clock[i].store(stats.GETGPUClockSpeed(i));
            }
            int diskCount = stats.GETDiskCount();
            statsData.DiskCount.store(diskCount);
            for (int i = 0; i < diskCount && i < 8; i++) {
                statsData.DiskUtil[i].store(stats.GETDiskUtilization(i));
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void renderLoop() {
        Config configManager;
        (void)configManager.load();
        const AppConfig& appCfg = configManager.get();

        int initW = appCfg.windowW > 0 ? appCfg.windowW : 1200;
        int initH = appCfg.windowH > 0 ? appCfg.windowH : 800;
        InitWindow(initW, initH, "Performance Monitor");
        SetWindowState(FLAG_WINDOW_RESIZABLE);
        SetWindowMinSize(500, 400);
        SetTargetFPS(30);

        if (appCfg.themeIndex >= 0 && appCfg.themeIndex <= 2) {
            selectedThemeIndex = appCfg.themeIndex;
            const Theme themes[] = {Theme::Dark(), Theme::Light(), Theme::HighContrast()};
            activeTheme = themes[appCfg.themeIndex];
        }
        for (int i = 0; i < 5; i++) tileEnabled[i] = appCfg.tileEnabled[i];
        if (appCfg.windowX != -1 && appCfg.windowY != -1) {
            SetWindowPosition(appCfg.windowX, appCfg.windowY);
        }

        Gauge::Dimensions gaugeDims;
        Gauge gaugeCPU(DesignSystem::makeGaugeTheme(activeTheme), gaugeDims, Gauge::Config::ConfigArc());
        Gauge gaugeRAM(DesignSystem::makeGaugeTheme(activeTheme), gaugeDims, Gauge::Config::ConfigQuarter());
        Gauge gaugeGPU(DesignSystem::makeGaugeTheme(activeTheme), gaugeDims, Gauge::Config::ConfigArc());

        Bar::Theme barTheme = DesignSystem::makeBarTheme(activeTheme);
        Bar::Dimensions barDims;
        Bar::Config barCfg;
        barCfg.autoScale = false;
        std::vector<Bar> bars;
        for (int i = 0; i < 16; i++) bars.emplace_back(barTheme, barDims, barCfg);

        std::vector<Tile> tiles = Tile::createDefaultTiles();
        const int gridCols = LayoutConfig::gridCols;
        const int gridRows = LayoutConfig::gridRows;

        while (!WindowShouldClose()) {
            int sw = GetScreenWidth();
            int sh = GetScreenHeight();
            float hScale = (std::max)((std::min)(sh / 800.0f, 1.8f), 0.6f);
            int fontSize = static_cast<int>((std::max)(18.0f * hScale, 10.0f));
            int titleSize = static_cast<int>(fontSize * 1.15f);

            if (appState == LANDING) {
                BeginDrawing();
                drawLandingPage(activeTheme, sw, sh, titleSize, fontSize, selectedThemeIndex, activeTheme, appState);
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

            int tileAreaH = sh - LayoutConfig::statusBarH;
            for (auto& t : tiles) t.computeBounds(sw, tileAreaH, gridCols, gridRows);

            Vector2 mousePos = GetMousePosition();
            for (auto& t : tiles) t.handleDrag(mousePos, sw, tileAreaH, tiles, gridCols, gridRows);

            StatsData local;
            loadStatsData(local);

            collectBarData(local, bars, gaugeCPU, gaugeRAM, gaugeGPU);

            if (loggingEnabled && logger.isLogging()) {
                logger.writeRow(local.CPU_Freq, local.CPU_Util, local.RAM_Used, local.RAM_Util,
                               local.GPU_Util[0], local.Wifi_Send, local.Wifi_Recv,
                               local.Ether_Send, local.Ether_Recv);
            }

            BeginDrawing();
            ClearBackground(activeTheme.windowBg);

            for (auto& t : tiles) {
                int idx = tileIndex(t);
                if (!tileEnabled[idx]) continue;
                float tfs = DesignSystem::tileTitleFont(t.bounds.width, t.bounds.height);
                t.drawFrame(activeTheme, tfs);
                renderTileContent(t, local, bars, gaugeCPU, gaugeRAM, gaugeGPU, fontSize);
            }

            for (auto& t : tiles) t.drawSnapPreview(activeTheme);

            DrawFPS(sw - 80, 12);
            drawStatusBar(sw, sh, fontSize);

            if (showDiagnostics) drawDiagnosticsOverlay(activeTheme, sw, sh, fontSize, stats, local);
            if (showSettings) drawSettingsOverlay(activeTheme, sw, sh, fontSize, stats, tileEnabled);

            EndDrawing();
        }

        dataRunning = false;
        saveConfig(configManager);
        logger.stop();
        CloseWindow();
    }

    void loadStatsData(StatsData& local) {
        local.CPU_Freq = statsData.CPU_Freq.load();
        local.CPU_Util = statsData.CPU_Util.load();
        local.RAM_Total = statsData.RAM_Total.load();
        local.RAM_Used = statsData.RAM_Used.load();
        local.RAM_Util = statsData.RAM_Util.load();
        local.Wifi_Send = statsData.Wifi_Send.load();
        local.Wifi_Recv = statsData.Wifi_Recv.load();
        local.Ether_Send = statsData.Ether_Send.load();
        local.Ether_Recv = statsData.Ether_Recv.load();
        local.cpuModel = statsData.cpuModel;
        local.gpuModel = statsData.gpuModel;
        local.GPUCount = statsData.GPUCount.load();
        for (int i = 0; i < local.GPUCount && i < 4; i++) {
            local.GPU_Util[i] = statsData.GPU_Util[i].load();
            local.GPU_VRAM[i] = statsData.GPU_VRAM[i].load();
            local.GPU_VRAMTotal[i] = statsData.GPU_VRAMTotal[i].load();
            local.GPU_Clock[i] = statsData.GPU_Clock[i].load();
        }
        local.DiskCount = statsData.DiskCount.load();
        for (int i = 0; i < local.DiskCount && i < 8; i++) {
            local.DiskUtil[i] = statsData.DiskUtil[i].load();
        }
    }

    void collectBarData(const StatsData& local, std::vector<Bar>& bars,
                        Gauge& gaugeCPU, Gauge& gaugeRAM, Gauge& gaugeGPU) {
        gaugeCPU.setValue(local.CPU_Util);
        gaugeRAM.setValue(local.RAM_Util);
        gaugeGPU.setValue(local.GPU_Util[0]);
        for (int i = 0; i < 16; i++) bars[i].setValue(0.0f);
        bars[0].setValue(local.CPU_Freq);
        bars[1].setValue(local.Wifi_Send);
        bars[2].setValue(local.Wifi_Recv);
        bars[3].setValue(local.Ether_Send);
        bars[4].setValue(local.Ether_Recv);
        bars[15].setValue(local.RAM_Util);
    }

    static int tileIndex(const Tile& tile) {
        if (tile.config.title == "CPU") return 0;
        if (tile.config.title == "RAM") return 1;
        if (tile.config.title == "GPU") return 2;
        if (tile.config.title == "Network") return 3;
        if (tile.config.title == "Storage") return 4;
        return -1;
    }

    void renderTileContent(Tile& tile, const StatsData& local, std::vector<Bar>& bars,
                           Gauge& gaugeCPU, Gauge& gaugeRAM, Gauge& gaugeGPU, int fontSize) {
        if (tile.config.title == "CPU")
            renderCPUTile(tile, activeTheme, local, stats, gaugeCPU, bars[0]);
        else if (tile.config.title == "RAM")
            renderRAMTile(tile, activeTheme, local, gaugeRAM, bars[15]);
        else if (tile.config.title == "GPU")
            renderGPUTile(tile, activeTheme, local, stats, bars, gaugeGPU);
        else if (tile.config.title == "Network")
            renderNetworkTile(tile, activeTheme, local, bars);
        else if (tile.config.title == "Storage")
            renderStorageTile(tile, activeTheme, local, stats, bars, fontSize);
    }

    void drawStatusBar(int sw, int sh, int fontSize) {
        int barH = LayoutConfig::statusBarH;
        Rectangle statusBg = {0, static_cast<float>(sh - barH), static_cast<float>(sw), static_cast<float>(barH)};
        DrawRectangleRec(statusBg, activeTheme.windowBg);
        DrawLineEx({0, static_cast<float>(sh - barH)}, {static_cast<float>(sw), static_cast<float>(sh - barH)},
                   1.0f, activeTheme.lineColor);
        int statusFont = (std::max)(fontSize - 4, 9);
        const char* logStatus = loggingEnabled ? "F4:Log [ON]" : "F4:Log [OFF]";
        Color logCol = loggingEnabled ? activeTheme.accentColor : activeTheme.textMuted;
        int logW = MeasureText(logStatus, statusFont);
        DrawText(logStatus, sw - logW - 12, sh - barH + 6, statusFont, logCol);
        DrawText("F2:Diag  F3:Settings  F4:Log", 12, sh - barH + 6, statusFont, activeTheme.textMuted);
    }

    void saveConfig(Config& configManager) {
        AppConfig saveCfg;
        saveCfg.themeIndex = selectedThemeIndex;
        for (int i = 0; i < 5; i++) saveCfg.tileEnabled[i] = tileEnabled[i];
        saveCfg.windowX = GetWindowPosition().x;
        saveCfg.windowY = GetWindowPosition().y;
        saveCfg.windowW = GetScreenWidth();
        saveCfg.windowH = GetScreenHeight();
        (void)configManager.save(saveCfg);
    }
};

#endif
