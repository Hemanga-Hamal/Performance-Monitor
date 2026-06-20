#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include <windows.h>
#include "raylib.h"
#include "StatsV1.h"
#include "BarV1.h"
#include "GaugeV1.h"
#include "ThemeV1.h"
#include "TileV1.h"
#include <thread>
#include <atomic>
#include <string>
#include <vector>

using RaylibVector2 = ::Vector2;
using RaylibColor = ::Color;

struct StatsData {
    std::atomic<float> CPU_Freq{0.0f};
    std::atomic<float> CPU_Util{0.0f};
    std::atomic<float> RAM_Util{0.0f};
    std::atomic<float> Wifi_Send{0.0f};
    std::atomic<float> Wifi_Recv{0.0f};
    std::atomic<float> Ether_Send{0.0f};
    std::atomic<float> Ether_Recv{0.0f};
    std::atomic<float> GPU_Util{0.0f};
    std::atomic<int> DiskCount{0};
    std::atomic<float> DiskUtil[8]{};
};

StatsData statsData;
std::atomic<bool> running(true);
StatsV1 stats1;

enum AppState { LANDING, DASHBOARD };
AppState appState = LANDING;
ThemeV1 activeTheme = ThemeV1::Dark();
int selectedThemeIndex = 0;
bool showDiagnostics = false;
bool showSettings = false;
bool tileEnabled[5] = {true, true, true, true, true}; // CPU, RAM, GPU, Network, Storage

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
        statsData.GPU_Util.store(stats1.GETGPUUtilization());

        int diskCount = stats1.GETDiskCount();
        statsData.DiskCount.store(diskCount);
        for (int i = 0; i < diskCount && i < 8; i++) {
            statsData.DiskUtil[i].store(stats1.GETDiskUtilization(i));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void drawLandingPage(int sw, int sh, int titleSize, int fontSize) {
    ClearBackground(activeTheme.landingBg);

    const char* title = "Performance Monitor";
    int titleW = MeasureText(title, titleSize * 2);
    DrawText(title, (sw - titleW) / 2, sh / 6, titleSize * 2, activeTheme.titleText);

    const char* subtitle = "Select a theme to get started";
    int subW = MeasureText(subtitle, fontSize);
    DrawText(subtitle, (sw - subW) / 2, sh / 6 + titleSize * 2 + 16, fontSize, activeTheme.textSecondary);

    const char* themes[] = {"Dark", "Light", "High Contrast"};
    ThemeV1 themePreviews[] = {ThemeV1::Dark(), ThemeV1::Light(), ThemeV1::HighContrast()};
    int cardW = 180, cardH = 120, spacing = 20;
    int startX = (sw - (cardW * 3 + spacing * 2)) / 2;
    int cardY = sh / 3 + 20;

    for (int i = 0; i < 3; i++) {
        Rectangle card = {static_cast<float>(startX + i * (cardW + spacing)),
                          static_cast<float>(cardY), static_cast<float>(cardW), static_cast<float>(cardH)};
        Color borderCol = (selectedThemeIndex == i) ? activeTheme.barForeground : activeTheme.tileBorder;

        DrawRectangleRounded(card, 0.1f, 8, themePreviews[i].tileBg);
        DrawRectangleRoundedLines(card, 0.1f, 8, borderCol);

        DrawText(themes[i], static_cast<int>(card.x + 16), static_cast<int>(card.y + 16),
                 fontSize, themePreviews[i].titleText);

        Rectangle s1 = {card.x + 16, card.y + 50, 40, 30};
        Rectangle s2 = {card.x + 66, card.y + 50, 40, 30};
        Rectangle s3 = {card.x + 116, card.y + 50, 40, 30};
        DrawRectangleRec(s1, themePreviews[i].barForeground);
        DrawRectangleRec(s2, themePreviews[i].gaugeArcActive);
        DrawRectangleRec(s3, themePreviews[i].barBackground);

        if (CheckCollisionPointRec(GetMousePosition(), card) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            selectedThemeIndex = i;
            activeTheme = themePreviews[i];
        }
    }

    int btnW = 200, btnH = 50;
    Rectangle startBtn = {static_cast<float>((sw - btnW) / 2), static_cast<float>(sh - sh / 5),
                          static_cast<float>(btnW), static_cast<float>(btnH)};
    DrawRectangleRounded(startBtn, 0.2f, 8, activeTheme.barForeground);
    const char* btnText = "Start Monitoring";
    int btnTextW = MeasureText(btnText, fontSize);
    DrawText(btnText, static_cast<int>(startBtn.x + (btnW - btnTextW) / 2),
             static_cast<int>(startBtn.y + (btnH - fontSize) / 2), fontSize, BLACK);

    if (CheckCollisionPointRec(GetMousePosition(), startBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        appState = DASHBOARD;
    }
}

void drawDiagnosticsOverlay(int sw, int sh, int fontSize) {
    static float scrollOffset = 0.0f;
    int panelW = sw * 4 / 5, panelH = sh * 4 / 5;
    if (panelW < 400) panelW = 400;
    if (panelH < 300) panelH = 300;
    Rectangle panel = {static_cast<float>((sw - panelW) / 2), static_cast<float>((sh - panelH) / 2),
                       static_cast<float>(panelW), static_cast<float>(panelH)};

    DrawRectangleRounded(panel, 0.05f, 8, {12, 12, 20, 248});
    DrawRectangleRoundedLines(panel, 0.05f, 8, activeTheme.tileBorder);

    int smallFont = std::max(fontSize - 3, 10);
    int sectionFont = std::max(fontSize - 1, 12);
    int x = static_cast<int>(panel.x + 16);
    int contentW = panelW - 32;
    int visibleTop = static_cast<int>(panel.y + 8);
    int visibleBot = static_cast<int>(panel.y + panelH - 8);

    DrawText("Diagnostics (F2 to close, scroll with mouse)", x, visibleTop, fontSize, WHITE);

    float wheel = GetMouseWheelMove();
    if (CheckCollisionPointRec(GetMousePosition(), panel)) {
        scrollOffset += wheel * 30.0f;
    }

    int y = visibleTop + fontSize + 16 + static_cast<int>(scrollOffset);

    // Helper: draw section header
    auto drawSection = [&](const char* title) {
        if (y > visibleTop && y < visibleBot) {
            DrawText(title, x, y, sectionFont, YELLOW);
        }
        y += sectionFont + 6;
    };

    // Helper: draw labeled line (label: value)
    auto drawLine = [&](const char* label, const char* value) {
        if (y > visibleTop && y < visibleBot) {
            DrawText(label, x, y, smallFont, activeTheme.textPrimary);
            int lw = MeasureText(label, smallFont);
            DrawText(value, x + lw + 8, y, smallFont, activeTheme.textSecondary);
        }
        y += smallFont + 3;
    };

    // CPU section
    drawSection("CPU");
    {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s", stats1.GETCPUModel());
        drawLine("Model:", buf);
        snprintf(buf, sizeof(buf), "%.0f MHz", stats1.GETCPUFrequency());
        drawLine("Frequency:", buf);
        snprintf(buf, sizeof(buf), "%.1f %%", stats1.GETCPUtilization());
        drawLine("Utilization:", buf);
    }
    y += 4;

    // RAM section
    drawSection("RAM");
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "%.1f GB", stats1.GETRAMTotal());
        drawLine("Total:", buf);
        snprintf(buf, sizeof(buf), "%.1f GB", stats1.GETRAMUsed());
        drawLine("Used:", buf);
        snprintf(buf, sizeof(buf), "%.1f %%", stats1.GETRAMUtilization());
        drawLine("Utilization:", buf);
    }
    y += 4;

    // GPU section
    drawSection("GPU");
    {
        char buf[256];
        if (stats1.IsGPUAvailable()) {
            snprintf(buf, sizeof(buf), "%s", stats1.GETGPUModel());
            drawLine("Model:", buf);
            char wbuf[128];
            WideCharToMultiByte(CP_UTF8, 0, stats1.GETGPUName(), -1, wbuf, sizeof(wbuf), nullptr, nullptr);
            snprintf(buf, sizeof(buf), "PDH: %s", wbuf);
            drawLine("Instance:", buf);
            snprintf(buf, sizeof(buf), "%.1f %%", stats1.GETGPUUtilization());
            drawLine("Utilization:", buf);
        } else {
            drawLine("Status:", "Not detected");
        }
    }
    y += 4;

    // Disk section
    drawSection("Disks");
    for (int i = 0; i < stats1.GETDiskCount(); i++) {
        if (y > visibleBot) break;
        char nameBuf[64], lineBuf[256];
        WideCharToMultiByte(CP_UTF8, 0, stats1.GETDiskName(i), -1, nameBuf, sizeof(nameBuf), nullptr, nullptr);
        snprintf(lineBuf, sizeof(lineBuf), "%.1f GB total, %.1f GB used, %.1f %%",
                 stats1.GETDiskTotal(i), stats1.GETDiskUsed(i), stats1.GETDiskUtilization(i));
        drawLine(nameBuf, lineBuf);
    }
    y += 4;

    // Network section
    drawSection("Network Interfaces");
    const auto& adapters = stats1.GetDiscoveredAdapters();
    if (adapters.empty()) {
        drawLine("Status:", "None detected");
    } else {
        for (const auto& a : adapters) {
            if (y > visibleBot) break;
            char wbuf[256];
            WideCharToMultiByte(CP_UTF8, 0, a.c_str(), -1, wbuf, sizeof(wbuf), nullptr, nullptr);
            drawLine("Adapter:", wbuf);
        }
    }

    // Scrollbar
    int totalH = y - static_cast<int>(scrollOffset) - visibleTop - fontSize - 16;
    if (totalH > panelH) {
        float barH = panelH * panelH / totalH;
        float barY = visibleTop + (-scrollOffset / totalH) * panelH;
        Rectangle sb = {panel.x + panelW - 10, barY, 6, barH};
        DrawRectangleRounded(sb, 0.5f, 4, activeTheme.textSecondary);
    }
}

void drawSettingsOverlay(int sw, int sh, int fontSize) {
    int panelW = 340, panelH = 320;
    Rectangle panel = {static_cast<float>(sw - panelW - 20), 50.0f,
                       static_cast<float>(panelW), static_cast<float>(panelH)};
    DrawRectangleRounded(panel, 0.06f, 8, {12, 12, 24, 240});
    DrawRectangleRoundedLines(panel, 0.06f, 8, activeTheme.tileBorder);
    DrawText("Settings (F3)", static_cast<int>(panel.x + 14), static_cast<int>(panel.y + 10), fontSize, WHITE);

    int y = static_cast<int>(panel.y + 40);
    int smallFont = std::max(fontSize - 2, 10);

    auto drawToggle = [&](const char* label, const char* info, int idx) {
        Rectangle cb = {panel.x + 14, static_cast<float>(y), 20, 20};
        DrawRectangleRec(cb, tileEnabled[idx] ? activeTheme.barForeground : activeTheme.tileBorder);
        DrawRectangleLinesEx(cb, 1.0f, WHITE);
        if (tileEnabled[idx]) {
            DrawText("x", static_cast<int>(cb.x + 5), static_cast<int>(cb.y + 1), smallFont, WHITE);
        }
        DrawText(label, static_cast<int>(cb.x + 28), y, smallFont, activeTheme.textPrimary);
        if (info[0]) {
            DrawText(info, static_cast<int>(cb.x + 28), y + smallFont + 2, smallFont - 2, activeTheme.textSecondary);
        }
        y += smallFont * 2 + 10;
        return CheckCollisionPointRec(GetMousePosition(), cb) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    };

    char cpuInfo[128] = "";
    snprintf(cpuInfo, sizeof(cpuInfo), "%s", stats1.GETCPUModel());
    if (drawToggle("CPU", cpuInfo, 0)) tileEnabled[0] = !tileEnabled[0];

    char ramInfo[64] = "";
    snprintf(ramInfo, sizeof(ramInfo), "%.1f GB total", stats1.GETRAMTotal());
    if (drawToggle("RAM", ramInfo, 1)) tileEnabled[1] = !tileEnabled[1];

    char gpuInfo[128] = "";
    if (stats1.IsGPUAvailable()) {
        char gbuf[64];
        WideCharToMultiByte(CP_UTF8, 0, stats1.GETGPUName(), -1, gbuf, sizeof(gbuf), nullptr, nullptr);
        snprintf(gpuInfo, sizeof(gpuInfo), "%s (%s)", stats1.GETGPUModel(), gbuf);
    } else {
        snprintf(gpuInfo, sizeof(gpuInfo), "Not detected");
    }
    if (drawToggle("GPU", gpuInfo, 2)) tileEnabled[2] = !tileEnabled[2];

    char netInfo[128] = "WiFi + Ethernet";
    const auto& adapters = stats1.GetDiscoveredAdapters();
    if (!adapters.empty()) {
        char abuf[128];
        WideCharToMultiByte(CP_UTF8, 0, adapters[0].c_str(), -1, abuf, sizeof(abuf), nullptr, nullptr);
        snprintf(netInfo, sizeof(netInfo), "%s", abuf);
    }
    if (drawToggle("Network", netInfo, 3)) tileEnabled[3] = !tileEnabled[3];

    char diskInfo[64] = "";
    int dc = stats1.GETDiskCount();
    snprintf(diskInfo, sizeof(diskInfo), "%d drive(s)", dc);
    if (drawToggle("Storage", diskInfo, 4)) tileEnabled[4] = !tileEnabled[4];
}

void renderLoop() {
    const int baseWidth = 1200, baseHeight = 800;
    InitWindow(baseWidth, baseHeight, "Performance Monitor");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(500, 400);
    SetTargetFPS(30);

    GaugeV1::Theme gaugeTheme;
    GaugeV1::Dimensions gaugeDims;
    GaugeV1 gaugeCPU(gaugeTheme, gaugeDims, GaugeV1::Config::ConfigArc());
    GaugeV1 gaugeRAM(gaugeTheme, gaugeDims, GaugeV1::Config::ConfigQuarter());

    BarV1::Theme barTheme;
    BarV1::Dimensions barDims;
    BarV1::Config barCfg;
    std::vector<BarV1> bars;
    for (int i = 0; i < 12; i++) bars.emplace_back(barTheme, barDims, barCfg);

    // Persistent tiles
    std::vector<TileV1> tiles;
    tiles.push_back(TileV1({0, 0, 1, 1, "CPU"}));
    tiles.push_back(TileV1({1, 0, 1, 1, "RAM"}));
    tiles.push_back(TileV1({0, 1, 1, 1, "GPU"}));
    tiles.push_back(TileV1({1, 1, 1, 1, "Network"}));
    tiles.push_back(TileV1({0, 2, 2, 2, "Storage"}));
    const int gridCols = 2;
    const int gridRows = 4;

    while (!WindowShouldClose()) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        float hScale = sh / static_cast<float>(baseHeight);
        int fontSize = std::max(static_cast<int>(18 * hScale), 10);
        int titleSize = static_cast<int>(fontSize * 1.3f);

        if (appState == LANDING) {
            BeginDrawing();
            drawLandingPage(sw, sh, titleSize, fontSize);
            EndDrawing();
            continue;
        }

        if (IsKeyPressed(KEY_F2)) showDiagnostics = !showDiagnostics;
        if (IsKeyPressed(KEY_F3)) showSettings = !showSettings;

        // Auto-layout tiles on first frame or screen resize
        for (auto& t : tiles) {
            t.computeBounds(sw, sh, gridCols, gridRows);
        }

        Vector2 mousePos = GetMousePosition();
        for (auto& t : tiles) {
            t.handleDrag(mousePos, sw, sh, tiles, gridCols, gridRows);
        }

        StatsData local;
        local.CPU_Freq = statsData.CPU_Freq.load();
        local.CPU_Util = statsData.CPU_Util.load();
        local.RAM_Util = statsData.RAM_Util.load();
        local.Wifi_Send = statsData.Wifi_Send.load();
        local.Wifi_Recv = statsData.Wifi_Recv.load();
        local.Ether_Send = statsData.Ether_Send.load();
        local.Ether_Recv = statsData.Ether_Recv.load();
        local.GPU_Util = statsData.GPU_Util.load();
        local.DiskCount = statsData.DiskCount.load();
        for (int i = 0; i < local.DiskCount && i < 8; i++) {
            local.DiskUtil[i] = statsData.DiskUtil[i].load();
        }

        gaugeCPU.setValue(local.CPU_Util);
        gaugeRAM.setValue(local.RAM_Util);
        for (int i = 0; i < 12; i++) bars[i].setValue(0.0f);
        bars[0].setValue(local.CPU_Freq);
        bars[1].setValue(local.Wifi_Send);
        bars[2].setValue(local.Wifi_Recv);
        bars[3].setValue(local.Ether_Send);
        bars[4].setValue(local.Ether_Recv);
        bars[5].setValue(local.GPU_Util);

        BeginDrawing();
        ClearBackground(activeTheme.windowBg);

        for (auto& tile : tiles) {
            int tileIdx = -1;
            if (tile.config.title == "CPU") tileIdx = 0;
            else if (tile.config.title == "RAM") tileIdx = 1;
            else if (tile.config.title == "GPU") tileIdx = 2;
            else if (tile.config.title == "Network") tileIdx = 3;
            else if (tile.config.title == "Storage") tileIdx = 4;

            if (!tileEnabled[tileIdx]) continue;

            tile.drawFrame(activeTheme, titleSize);

            if (tile.config.title == "CPU") {
                Vector2 c = tile.contentCenter();
                float cw = tile.contentWidth();
                float ch = tile.contentHeight();
                float gaugeSize = std::min(cw * 0.5f, ch * 0.45f);
                gaugeCPU.setAutoScale(false);
                gaugeCPU.setBaseSize(gaugeSize);
                const char* cpuModel = stats1.GETCPUModel();
                int smallFont = std::max(fontSize - 2, 8);
                if (cpuModel[0]) {
                    float modelY = tile.bounds.y + 30.0f + 6.0f;
                    DrawText(cpuModel, static_cast<int>(c.x - MeasureText(cpuModel, smallFont) / 2),
                             static_cast<int>(modelY), smallFont, activeTheme.textSecondary);
                }
                gaugeCPU.draw({c.x, c.y + ch * 0.05f}, "Utilization");
                barDims.maxSize = cw * 0.8f;
                bars[0].setDimensions(barDims);
                bars[0].draw({c.x, c.y + ch * 0.35f}, "Frequency (MHz)", formatValue(local.CPU_Freq));
            }
            else if (tile.config.title == "RAM") {
                Vector2 c = tile.contentCenter();
                float cw = tile.contentWidth();
                float ch = tile.contentHeight();
                float gaugeSize = std::min(cw * 0.5f, ch * 0.5f);
                gaugeRAM.setAutoScale(false);
                gaugeRAM.setBaseSize(gaugeSize);
                gaugeRAM.draw({c.x, c.y}, "Load");
            }
            else if (tile.config.title == "GPU") {
                Vector2 c = tile.contentCenter();
                float cw = tile.contentWidth();
                float ch = tile.contentHeight();
                const char* gpuModel = stats1.GETGPUModel();
                int smallFont = std::max(fontSize - 2, 8);
                if (gpuModel[0]) {
                    float modelY = tile.bounds.y + 30.0f + 6.0f;
                    DrawText(gpuModel, static_cast<int>(c.x - MeasureText(gpuModel, smallFont) / 2),
                             static_cast<int>(modelY), smallFont, activeTheme.textSecondary);
                }
                barDims.maxSize = cw * 0.8f;
                bars[5].setDimensions(barDims);
                bars[5].draw({c.x, c.y + ch * 0.15f}, "Utilization %", formatValue(local.GPU_Util));
            }
            else if (tile.config.title == "Network") {
                Vector2 c = tile.contentCenter();
                float cw = tile.contentWidth();
                float sp = tile.contentHeight() * 0.16f;
                barDims.maxSize = cw * 0.85f;
                bars[1].setDimensions(barDims);
                bars[2].setDimensions(barDims);
                bars[3].setDimensions(barDims);
                bars[4].setDimensions(barDims);
                bars[1].draw({c.x, c.y - sp * 1.5f}, "WiFi Send (Mbps)", formatValue(local.Wifi_Send));
                bars[2].draw({c.x, c.y - sp * 0.5f}, "WiFi Recv (Mbps)", formatValue(local.Wifi_Recv));
                bars[3].draw({c.x, c.y + sp * 0.5f}, "Eth Send (Mbps)", formatValue(local.Ether_Send));
                bars[4].draw({c.x, c.y + sp * 1.5f}, "Eth Recv (Mbps)", formatValue(local.Ether_Recv));
            }
            else if (tile.config.title == "Storage") {
                int diskCount = local.DiskCount.load();
                if (diskCount < 0) diskCount = 0;
                if (diskCount == 0) {
                    Vector2 c = tile.contentCenter();
                    DrawText("No fixed disks found", static_cast<int>(c.x - MeasureText("No fixed disks found", fontSize) / 2),
                             static_cast<int>(c.y - fontSize / 2), fontSize, activeTheme.textSecondary);
                } else {
                    Vector2 c = tile.contentCenter();
                    float cw = tile.contentWidth();
                    float ch = tile.contentHeight();
                    float usableH = ch * 0.75f;
                    float barH = usableH / static_cast<float>(diskCount);
                    if (barH > 45.0f) barH = 45.0f;
                    float startY = c.y - (diskCount * barH) / 2 + barH / 2;
                    barDims.maxSize = cw * 0.8f;
                    for (int d = 0; d < diskCount && d < 8; d++) {
                        char buf[64];
                        WideCharToMultiByte(CP_UTF8, 0, stats1.GETDiskName(d), -1, buf, sizeof(buf), nullptr, nullptr);
                        bars[6 + d].setValue(local.DiskUtil[d]);
                        bars[6 + d].setDimensions(barDims);
                        bars[6 + d].draw({c.x, startY + d * barH}, buf, formatValue(local.DiskUtil[d]));
                    }
                }
            }
        }

        // Draw snap previews on top
        for (auto& t : tiles) {
            t.drawSnapPreview(activeTheme);
        }

        DrawFPS(sw - 90, 8);
        if (showDiagnostics) drawDiagnosticsOverlay(sw, sh, fontSize);
        if (showSettings) drawSettingsOverlay(sw, sh, fontSize);

        const char* hint = showDiagnostics ? "F2:Diag F3:Settings" : "F2:Diag F3:Settings";
        DrawText(hint, 8, sh - 24, std::max(fontSize - 4, 10), activeTheme.textSecondary);
        EndDrawing();
    }

    running = false;
    CloseWindow();
}

int main() {
    std::thread dataThread(updateStats);
    std::thread renderThread(renderLoop);
    dataThread.join();
    renderThread.join();
    return 0;
}
