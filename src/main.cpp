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
#include "ConfigV1.h"
#include "LoggerV1.h"
#include <cstdio>
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
    std::atomic<float> GPU_Util[4]{};
    std::atomic<int> GPUCount{0};
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
bool tileEnabled[5] = {true, true, true, true, true};
bool loggingEnabled = false;
LoggerV1 logger;

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

void drawLandingPage(int sw, int sh, int titleSize, int fontSize) {
    ClearBackground(activeTheme.landingBg);

    int heroY = sh / 5;
    const char* title = "Performance Monitor";
    int titleFont = titleSize * 2;
    int titleW = MeasureText(title, titleFont);
    DrawText(title, (sw - titleW) / 2, heroY, titleFont, activeTheme.titleText);

    const char* subtitle = "Select a theme to get started";
    int subFont = static_cast<int>(fontSize * 0.95f);
    int subW = MeasureText(subtitle, subFont);
    DrawText(subtitle, (sw - subW) / 2, heroY + titleFont + 20, subFont, activeTheme.textSecondary);

    const char* themeNames[] = {"Dark", "Light", "High Contrast"};
    ThemeV1 themePreviews[] = {ThemeV1::Dark(), ThemeV1::Light(), ThemeV1::HighContrast()};

    float cardScale = std::min(sw / 1300.0f, sh / 850.0f);
    cardScale = std::clamp(cardScale, 0.5f, 2.0f);
    int cardW = static_cast<int>(200 * cardScale);
    int cardH = static_cast<int>(140 * cardScale);
    int spacing = static_cast<int>(24 * cardScale);
    int totalCardsW = cardW * 3 + spacing * 2;
    int startX = (sw - totalCardsW) / 2;
    int cardY = heroY + titleFont + 80;

    for (int i = 0; i < 3; i++) {
        Rectangle card = {static_cast<float>(startX + i * (cardW + spacing)),
                          static_cast<float>(cardY), static_cast<float>(cardW), static_cast<float>(cardH)};
        bool isSelected = (selectedThemeIndex == i);
        bool isHovered = CheckCollisionPointRec(GetMousePosition(), card);

        Rectangle shadow = {card.x + 4, card.y + 4, card.width, card.height};
        DrawRectangleRounded(shadow, 0.12f, 12, activeTheme.tileShadow);

        DrawRectangleRounded(card, 0.12f, 12, themePreviews[i].tileBg);

        Color borderCol = isSelected ? activeTheme.accentColor :
                          (isHovered ? activeTheme.accentHover : themePreviews[i].tileBorder);
        float borderW = isSelected ? 2.5f : 1.0f;
        DrawRectangleRoundedLinesEx(card, 0.12f, 12, borderW, borderCol);

        if (isSelected) {
            Rectangle glow = {card.x - 1, card.y - 1, card.width + 2, card.height + 2};
            DrawRectangleRoundedLinesEx(glow, 0.12f, 12, 3.0f,
                {activeTheme.accentColor.r, activeTheme.accentColor.g, activeTheme.accentColor.b, 40});
        }

        int tf = static_cast<int>(fontSize * cardScale * 0.9f);
        if (tf < 11) tf = 11;
        DrawText(themeNames[i], static_cast<int>(card.x + 18 * cardScale),
                 static_cast<int>(card.y + 16 * cardScale), tf, themePreviews[i].titleText);

        float swatchW = 44 * cardScale;
        float swatchH = 32 * cardScale;
        float swatchGap = 12 * cardScale;
        float sy = card.y + 50 * cardScale;
        float sx = card.x + 18 * cardScale;
        Rectangle s1 = {sx, sy, swatchW, swatchH};
        Rectangle s2 = {sx + swatchW + swatchGap, sy, swatchW, swatchH};
        Rectangle s3 = {sx + (swatchW + swatchGap) * 2, sy, swatchW, swatchH};
        DrawRectangleRounded(s1, 0.2f, 8, themePreviews[i].barForeground);
        DrawRectangleRounded(s2, 0.2f, 8, themePreviews[i].gaugeArcActive);
        DrawRectangleRounded(s3, 0.2f, 8, themePreviews[i].barBackground);

        if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            selectedThemeIndex = i;
            activeTheme = themePreviews[i];
        }
    }

    int btnW = static_cast<int>(220 * cardScale);
    int btnH = static_cast<int>(52 * cardScale);
    if (btnH < 34) btnH = 34;
    Rectangle startBtn = {static_cast<float>((sw - btnW) / 2),
                          static_cast<float>(sh - sh / 4),
                          static_cast<float>(btnW), static_cast<float>(btnH)};
    bool btnHover = CheckCollisionPointRec(GetMousePosition(), startBtn);
    Color btnColor = btnHover ? activeTheme.accentHover : activeTheme.accentColor;

    Rectangle btnShadow = {startBtn.x + 2, startBtn.y + 3, startBtn.width, startBtn.height};
    DrawRectangleRounded(btnShadow, 0.25f, 12, {0, 0, 0, 50});
    DrawRectangleRounded(startBtn, 0.25f, 12, btnColor);

    const char* btnText = "Start Monitoring";
    int btnFont = static_cast<int>(fontSize * cardScale * 0.95f);
    if (btnFont < 12) btnFont = 12;
    int btnTextW = MeasureText(btnText, btnFont);
    DrawText(btnText, static_cast<int>(startBtn.x + (btnW - btnTextW) / 2),
             static_cast<int>(startBtn.y + (btnH - btnFont) / 2), btnFont,
             activeTheme.windowBg);

    if (btnHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        appState = DASHBOARD;
    }
}

void drawDiagnosticsOverlay(int sw, int sh, int fontSize) {
    static float scrollOffset = 0.0f;
    int panelW = sw * 4 / 5, panelH = sh * 4 / 5;
    if (panelW < 420) panelW = 420;
    if (panelH < 320) panelH = 320;
    Rectangle panel = {static_cast<float>((sw - panelW) / 2), static_cast<float>((sh - panelH) / 2),
                       static_cast<float>(panelW), static_cast<float>(panelH)};

    Rectangle shadow = {panel.x + 4, panel.y + 6, panel.width, panel.height};
    DrawRectangleRounded(shadow, 0.08f, 16, {0, 0, 0, 80});
    DrawRectangleRounded(panel, 0.08f, 16, activeTheme.panelOverlay);
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 1.0f, activeTheme.tileBorder);

    int smallFont = std::max(fontSize - 3, 10);
    int sectionFont = std::max(fontSize - 1, 12);
    int x = static_cast<int>(panel.x + 20);
    int visibleTop = static_cast<int>(panel.y + 12);
    int visibleBot = static_cast<int>(panel.y + panelH - 12);

    DrawText("Diagnostics", x, visibleTop, fontSize, activeTheme.titleText);

    int saveW = std::min(100, panelW - 200);
    if (saveW > 55) {
        Rectangle saveBtn = {panel.x + panelW - saveW - 14, panel.y + 10, static_cast<float>(saveW), 26};
        bool saveHover = CheckCollisionPointRec(GetMousePosition(), saveBtn);
        Color sbBg = saveHover ? activeTheme.accentColor : activeTheme.toggleInactive;
        DrawRectangleRounded(saveBtn, 0.3f, 8, sbBg);
        const char* saveText = "Save";
        int tw = MeasureText(saveText, smallFont);
        DrawText(saveText, static_cast<int>(saveBtn.x + (saveW - tw) / 2),
                 static_cast<int>(saveBtn.y + 5), smallFont, WHITE);
        if (saveHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            FILE* f = nullptr;
            _wfopen_s(&f, L"diagnostics.txt", L"w, ccs=UTF-8");
            if (f) {
                fwprintf(f, L"=== Performance Monitor Diagnostics ===\n\n");
                fwprintf(f, L"[CPU]\n");
                fwprintf(f, L"  Model: %hs\n", stats1.GETCPUModel());
                fwprintf(f, L"  Frequency: %.0f MHz\n", stats1.GETCPUFrequency());
                fwprintf(f, L"  Utilization: %.1f %%\n", stats1.GETCPUtilization());
                fwprintf(f, L"\n[RAM]\n");
                fwprintf(f, L"  Total: %.1f GB\n", stats1.GETRAMTotal());
                fwprintf(f, L"  Used: %.1f GB\n", stats1.GETRAMUsed());
                fwprintf(f, L"  Utilization: %.1f %%\n", stats1.GETRAMUtilization());
                fwprintf(f, L"\n[GPU]\n");
                fwprintf(f, L"  Model: %hs\n", stats1.GETGPUModel());
                int gc = stats1.GETGPUCount();
                for (int i = 0; i < gc; i++) {
                    char nbuf[128];
                    WideCharToMultiByte(CP_UTF8, 0, stats1.GETGPUName(i), -1, nbuf, sizeof(nbuf), nullptr, nullptr);
                    fwprintf(f, L"  %hs: %.1f %%\n", nbuf, stats1.GETGPUUtilization(i));
                }
                fwprintf(f, L"\n[Disks]\n");
                for (int i = 0; i < stats1.GETDiskCount(); i++) {
                    char nbuf[64];
                    WideCharToMultiByte(CP_UTF8, 0, stats1.GETDiskName(i), -1, nbuf, sizeof(nbuf), nullptr, nullptr);
                    fwprintf(f, L"  %hs: %.1f GB / %.1f GB (%.1f %%)\n",
                             nbuf, stats1.GETDiskUsed(i), stats1.GETDiskTotal(i), stats1.GETDiskUtilization(i));
                }
                fwprintf(f, L"\n[Network]\n");
                const auto& adps = stats1.GetAdapters();
                for (const auto& a : adps) {
                    char nbuf[128];
                    WideCharToMultiByte(CP_UTF8, 0, a.name.c_str(), -1, nbuf, sizeof(nbuf), nullptr, nullptr);
                    fwprintf(f, L"  %hs\n", nbuf);
                }
                fwprintf(f, L"\n");
                fclose(f);
            }
        }
    }

    float wheel = GetMouseWheelMove();
    if (CheckCollisionPointRec(GetMousePosition(), panel)) {
        scrollOffset += wheel * 30.0f;
    }
    int y = visibleTop + fontSize + 20 + static_cast<int>(scrollOffset);

    int clipY = static_cast<int>(panel.y + 48);
    int clipH = static_cast<int>(panelH - 56);
    if (clipH < 0) clipH = 0;
    BeginScissorMode(static_cast<int>(panel.x + 2), clipY, panelW - 4, clipH);

    auto drawSection = [&](const char* title) {
        if (y > visibleTop - 20 && y < visibleBot + 20) {
            float secX = panel.x + 20;
            DrawText(title, static_cast<int>(secX), y, sectionFont, activeTheme.accentColor);
            float lineW = std::min(panelW - 100.0f, 400.0f);
            DrawLineEx({secX + MeasureText(title, sectionFont) + 10, y + sectionFont / 2.0f},
                       {secX + lineW, y + sectionFont / 2.0f},
                       1.0f, activeTheme.lineColor);
        }
        y += sectionFont + 8;
    };

    auto drawLine = [&](const char* label, const char* value) {
        if (y > visibleTop - 20 && y < visibleBot + 20) {
            float lx = panel.x + 28;
            DrawText(label, static_cast<int>(lx), y, smallFont, activeTheme.textPrimary);
            int lw = MeasureText(label, smallFont);
            float vx = lx + lw + 10;
            float maxVW = panel.x + panelW - vx - 20.0f;
            int vw = MeasureText(value, smallFont);
            if (vw > maxVW && maxVW > 30.0f) {
                int smallerVFont = smallFont;
                while (smallerVFont > 8) {
                    smallerVFont--;
                    vw = MeasureText(value, smallerVFont);
                    if (vw <= maxVW) break;
                }
                DrawText(value, static_cast<int>(vx), y, smallerVFont, activeTheme.textSecondary);
            } else {
                DrawText(value, static_cast<int>(vx), y, smallFont, activeTheme.textSecondary);
            }
        }
        y += smallFont + 4;
    };

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
    y += 6;

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
    y += 6;

    drawSection("GPU");
    {
        char buf[256];
        if (stats1.IsGPUAvailable()) {
            snprintf(buf, sizeof(buf), "%s", stats1.GETGPUModel());
            drawLine("Model:", buf);
            int gpuCount = stats1.GETGPUCount();
            for (int i = 0; i < gpuCount; i++) {
                char wbuf[128], lineBuf[256];
                WideCharToMultiByte(CP_UTF8, 0, stats1.GETGPUName(i), -1, wbuf, sizeof(wbuf), nullptr, nullptr);
                snprintf(lineBuf, sizeof(lineBuf), "%.1f %%", stats1.GETGPUUtilization(i));
                drawLine(wbuf, lineBuf);
            }
        } else {
            drawLine("Status:", "Not detected");
        }
    }
    y += 6;

    drawSection("Disks");
    for (int i = 0; i < stats1.GETDiskCount(); i++) {
        if (y > visibleBot) break;
        char nameBuf[64], lineBuf[256];
        WideCharToMultiByte(CP_UTF8, 0, stats1.GETDiskName(i), -1, nameBuf, sizeof(nameBuf), nullptr, nullptr);
        snprintf(lineBuf, sizeof(lineBuf), "%.1f GB total, %.1f GB used, %.1f %%",
                 stats1.GETDiskTotal(i), stats1.GETDiskUsed(i), stats1.GETDiskUtilization(i));
        drawLine(nameBuf, lineBuf);
    }
    y += 6;

    drawSection("Network");
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

    int totalH = y - static_cast<int>(scrollOffset) - visibleTop - fontSize - 20;
    if (totalH > panelH) {
        float barH = panelH * panelH / static_cast<float>(totalH);
        float barY = visibleTop + 48.0f + (-scrollOffset / totalH) * (panelH - 56.0f);
        Rectangle sb = {panel.x + panelW - 8, barY, 4, barH};
        DrawRectangleRounded(sb, 0.5f, 6, activeTheme.scrollbarColor);
    }
    EndScissorMode();
}

void drawSettingsOverlay(int sw, int sh, int fontSize) {
    static float settingsScroll = 0.0f;
    int panelW = 370, panelH = sh * 4 / 5;
    if (panelH < 320) panelH = 320;
    Rectangle panel = {static_cast<float>(sw - panelW - 20), 50.0f,
                       static_cast<float>(panelW), static_cast<float>(panelH)};

    Rectangle shadow = {panel.x + 3, panel.y + 5, panel.width, panel.height};
    DrawRectangleRounded(shadow, 0.08f, 16, {0, 0, 0, 70});
    DrawRectangleRounded(panel, 0.08f, 16, activeTheme.panelOverlay);
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 1.0f, activeTheme.tileBorder);
    DrawText("Settings", static_cast<int>(panel.x + 20), static_cast<int>(panel.y + 14), fontSize, activeTheme.titleText);

    float wheel = GetMouseWheelMove();
    if (CheckCollisionPointRec(GetMousePosition(), panel)) {
        settingsScroll += wheel * 30.0f;
    }

    int smallFont = std::max(fontSize - 2, 10);
    int visibleTop = static_cast<int>(panel.y + 12);
    int visibleBot = static_cast<int>(panel.y + panelH - 12);
    int yBase = static_cast<int>(panel.y + 44 + settingsScroll);

    int clipY = static_cast<int>(panel.y + 42);
    int clipH = static_cast<int>(panelH - 50);
    if (clipH < 0) clipH = 0;
    BeginScissorMode(static_cast<int>(panel.x + 2), clipY, panelW - 4, clipH);

    auto drawToggle = [&](const char* label, const char* info, bool& enabled, int& y) {
        float cbX = panel.x + 18;
        Rectangle cb = {cbX, static_cast<float>(y + 1), 20, 20};
        if (y + 42 > visibleTop && y < visibleBot) {
            Color bg = enabled ? activeTheme.toggleActive : activeTheme.toggleInactive;
            DrawRectangleRounded(cb, 0.25f, 8, bg);
            DrawRectangleRoundedLinesEx(cb, 0.25f, 8, 1.0f, activeTheme.tileBorder);
            if (enabled) {
                const char* check = "\xE2\x9C\x93";
                int cw = MeasureText(check, smallFont);
                DrawText(check, static_cast<int>(cb.x + (20 - cw) / 2), y + 2, smallFont, WHITE);
            }
            float lx = cbX + 30;
            float maxTextW = panel.x + panelW - lx - 20.0f;
            DrawText(label, static_cast<int>(lx), y, smallFont, activeTheme.textPrimary);
            if (info[0]) {
                int infoFont = smallFont - 3;
                if (infoFont < 8) infoFont = 8;
                int iw = MeasureText(info, infoFont);
                if (iw > maxTextW && maxTextW > 20.0f) {
                    while (infoFont > 8) { infoFont--; iw = MeasureText(info, infoFont); if (iw <= maxTextW) break; }
                }
                DrawText(info, static_cast<int>(lx), y + smallFont + 3, infoFont, activeTheme.textSecondary);
            }
        }
        y += smallFont * 2 + 14;
        return CheckCollisionPointRec(GetMousePosition(), cb) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    };

    int y = yBase;

    {
        char cpuInfo[128] = "";
        snprintf(cpuInfo, sizeof(cpuInfo), "%s", stats1.GETCPUModel());
        if (drawToggle("CPU Tile", cpuInfo, tileEnabled[0], y)) tileEnabled[0] = !tileEnabled[0];

        char ramInfo[64] = "";
        snprintf(ramInfo, sizeof(ramInfo), "%.1f GB total", stats1.GETRAMTotal());
        if (drawToggle("RAM Tile", ramInfo, tileEnabled[1], y)) tileEnabled[1] = !tileEnabled[1];

        char gpuInfo[128] = "";
        if (stats1.IsGPUAvailable()) {
            int gpuCount = stats1.GETGPUCount();
            if (gpuCount > 1) {
                snprintf(gpuInfo, sizeof(gpuInfo), "%s (%d GPUs)", stats1.GETGPUModel(), gpuCount);
            } else {
                char gbuf[64];
                WideCharToMultiByte(CP_UTF8, 0, stats1.GETGPUName(0), -1, gbuf, sizeof(gbuf), nullptr, nullptr);
                snprintf(gpuInfo, sizeof(gpuInfo), "%s (%s)", stats1.GETGPUModel(), gbuf);
            }
        } else {
            snprintf(gpuInfo, sizeof(gpuInfo), "Not detected");
        }
        if (drawToggle("GPU Tile", gpuInfo, tileEnabled[2], y)) tileEnabled[2] = !tileEnabled[2];

        if (drawToggle("Network Tile", "WiFi + Ethernet", tileEnabled[3], y)) tileEnabled[3] = !tileEnabled[3];

        char diskInfo[64] = "";
        int dc = stats1.GETDiskCount();
        snprintf(diskInfo, sizeof(diskInfo), "%d drive(s)", dc);
        if (drawToggle("Storage Tile", diskInfo, tileEnabled[4], y)) tileEnabled[4] = !tileEnabled[4];
    }

    y += 10;

    const auto& disks = stats1.GetDisks();
    if (!disks.empty()) {
        float secX = panel.x + 18;
        if (y + smallFont + 8 > visibleTop && y < visibleBot)
            DrawText("Disks", static_cast<int>(secX), y, smallFont, activeTheme.accentColor);
        y += smallFont + 8;
        for (size_t i = 0; i < disks.size(); i++) {
            char label[64], info[128];
            WideCharToMultiByte(CP_UTF8, 0, disks[i].name.c_str(), -1, label, sizeof(label), nullptr, nullptr);
            snprintf(info, sizeof(info), "%.1f GB total", disks[i].totalGB);
            bool enabled = disks[i].enabled;
            if (drawToggle(label, info, enabled, y)) {
                stats1.SetDiskEnabled(static_cast<int>(i), !enabled);
            }
        }
        y += 10;
    }

    const auto& adapters = stats1.GetAdapters();
    if (!adapters.empty()) {
        float secX = panel.x + 18;
        if (y + smallFont + 8 > visibleTop && y < visibleBot)
            DrawText("Network", static_cast<int>(secX), y, smallFont, activeTheme.accentColor);
        y += smallFont + 8;
        for (size_t i = 0; i < adapters.size(); i++) {
            char label[128], info[64];
            WideCharToMultiByte(CP_UTF8, 0, adapters[i].name.c_str(), -1, label, sizeof(label), nullptr, nullptr);
            snprintf(info, sizeof(info), "%s", adapters[i].isWiFi ? "WiFi" : (adapters[i].isEthernet ? "Ethernet" : "Other"));
            bool enabled = adapters[i].enabled;
            if (drawToggle(label, info, enabled, y)) {
                stats1.SetAdapterEnabled(static_cast<int>(i), !enabled);
            }
        }
    }

    EndScissorMode();
}

void renderLoop() {
    ConfigV1 configManager;
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
        const ThemeV1 themes[] = {ThemeV1::Dark(), ThemeV1::Light(), ThemeV1::HighContrast()};
        activeTheme = themes[appCfg.themeIndex];
    }
    for (int i = 0; i < 5; i++) {
        tileEnabled[i] = appCfg.tileEnabled[i];
    }
    if (appCfg.windowX != -1 && appCfg.windowY != -1) {
        SetWindowPosition(appCfg.windowX, appCfg.windowY);
    }

    GaugeV1::Theme gaugeTheme;
    GaugeV1::Dimensions gaugeDims;
    GaugeV1 gaugeCPU(gaugeTheme, gaugeDims, GaugeV1::Config::ConfigArc());
    GaugeV1 gaugeRAM(gaugeTheme, gaugeDims, GaugeV1::Config::ConfigQuarter());

    BarV1::Theme barTheme;
    BarV1::Dimensions barDims;
    BarV1::Config barCfg;
    std::vector<BarV1> bars;
    for (int i = 0; i < 16; i++) bars.emplace_back(barTheme, barDims, barCfg);

    std::vector<TileV1> tiles;
    tiles.push_back(TileV1({0, 0, 2, 2, "CPU"}));
    tiles.push_back(TileV1({2, 0, 2, 2, "RAM"}));
    tiles.push_back(TileV1({0, 2, 2, 2, "GPU"}));
    tiles.push_back(TileV1({2, 2, 2, 2, "Network"}));
    tiles.push_back(TileV1({0, 4, 4, 2, "Storage"}));
    const int gridCols = 4;
    const int gridRows = 6;

    const float baseHeight = 800.0f;

    while (!WindowShouldClose()) {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        float hScale = std::clamp(sh / 800.0f, 0.6f, 1.8f);
        int fontSize = static_cast<int>(std::max(18.0f * hScale, 10.0f));
        int titleSize = static_cast<int>(fontSize * 1.15f);

        if (appState == LANDING) {
            BeginDrawing();
            drawLandingPage(sw, sh, titleSize, fontSize);
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

        int tileAreaH = sh - 30;
        for (auto& t : tiles) {
            t.computeBounds(sw, tileAreaH, gridCols, gridRows);
        }

        Vector2 mousePos = GetMousePosition();
        for (auto& t : tiles) {
            t.handleDrag(mousePos, sw, tileAreaH, tiles, gridCols, gridRows);
        }

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

        for (auto& tile : tiles) {
            int tileIdx = -1;
            if (tile.config.title == "CPU") tileIdx = 0;
            else if (tile.config.title == "RAM") tileIdx = 1;
            else if (tile.config.title == "GPU") tileIdx = 2;
            else if (tile.config.title == "Network") tileIdx = 3;
            else if (tile.config.title == "Storage") tileIdx = 4;

            if (!tileEnabled[tileIdx]) continue;

            float tileTitleSize = std::min(tile.bounds.width * 0.08f, tile.bounds.height * 0.09f);
            if (tileTitleSize < 10.0f) tileTitleSize = 10.0f;
            if (tileTitleSize > 24.0f) tileTitleSize = 24.0f;
            tile.drawFrame(activeTheme, tileTitleSize);

            if (tile.config.title == "CPU") {
                Rectangle tb = tile.titleBar();
                float contentTop = tb.y + tb.height;
                float contentBot = tile.bounds.y + tile.bounds.height - 8.0f;
                float contentH = contentBot - contentTop;
                float cw = tile.contentWidth();
                float midX = tile.bounds.x + tile.bounds.width / 2;

                const char* cpuModel = stats1.GETCPUModel();
                float modelFont = std::min(tile.bounds.width * 0.04f, 16.0f);
                if (modelFont < 8.0f) modelFont = 8.0f;
                bool hasModel = cpuModel[0] != '\0';
                if (hasModel) {
                    int mw = MeasureText(cpuModel, static_cast<int>(modelFont));
                    float maxW = tile.bounds.width - 16.0f;
                    float useFont = modelFont;
                    while (useFont > 7.0f && mw > maxW) { useFont -= 0.5f; mw = MeasureText(cpuModel, static_cast<int>(useFont)); }
                    DrawText(cpuModel, static_cast<int>(midX - mw / 2),
                             static_cast<int>(contentTop + 4.0f), static_cast<int>(useFont), activeTheme.textSecondary);
                }
                float gaugeTop = contentTop + (hasModel ? modelFont + 8.0f : 6.0f);
                float gaugeAvail = contentH * 0.60f;
                float gaugeSize = std::min(cw * 0.58f, gaugeAvail * 0.80f);
                if (gaugeSize < 40.0f) gaugeSize = 40.0f;
                gaugeCPU.setAutoScale(false);
                gaugeCPU.setBaseSize(gaugeSize);
                float gaugeCenterY = gaugeTop + gaugeSize * 0.52f;
                gaugeCPU.draw({midX, gaugeCenterY}, "Utilization");

                float gaugeBottom = gaugeCenterY + gaugeSize * 0.42f;
                barDims.maxSize = cw * 0.88f;
                bars[0].setDimensions(barDims);
                float barTotalH = bars[0].getTotalHeight();
                float barCenterY = contentBot - barTotalH * 0.45f;
                if (barCenterY < gaugeBottom + barTotalH * 0.5f + 6.0f) {
                    barCenterY = gaugeBottom + barTotalH * 0.5f + 6.0f;
                }
                if (barCenterY + barTotalH * 0.5f > contentBot) barCenterY = contentBot - barTotalH * 0.5f - 2.0f;
                bars[0].draw({midX, barCenterY}, "Frequency", formatValue(local.CPU_Freq));
            }
            else if (tile.config.title == "RAM") {
                Rectangle tb = tile.titleBar();
                float contentTop = tb.y + tb.height;
                float contentBot = tile.bounds.y + tile.bounds.height - 8.0f;
                float contentH = contentBot - contentTop;
                float cw = tile.contentWidth();
                float midX = tile.bounds.x + tile.bounds.width / 2;

                float gaugeAvail = contentH * 0.80f;
                float gaugeSize = std::min(cw * 0.48f, gaugeAvail * 0.68f);
                if (gaugeSize < 30.0f) gaugeSize = 30.0f;
                gaugeRAM.setAutoScale(false);
                gaugeRAM.setBaseSize(gaugeSize);
                float gaugeCenterY = contentTop + gaugeAvail * 0.40f;
                gaugeRAM.draw({midX, gaugeCenterY}, "Load");
            }
            else if (tile.config.title == "GPU") {
                Rectangle tb = tile.titleBar();
                float contentTop = tb.y + tb.height;
                float contentBot = tile.bounds.y + tile.bounds.height - 8.0f;
                float contentH = contentBot - contentTop;
                float cw = tile.contentWidth();
                float midX = tile.bounds.x + tile.bounds.width / 2;

                const char* gpuModel = stats1.GETGPUModel();
                float modelFont = std::min(tile.bounds.width * 0.04f, 16.0f);
                if (modelFont < 8.0f) modelFont = 8.0f;
                bool hasModel = gpuModel[0] != '\0';
                if (hasModel) {
                    int mw = MeasureText(gpuModel, static_cast<int>(modelFont));
                    float maxW = tile.bounds.width - 16.0f;
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
                    if (gpuCount == 1) {
                        snprintf(gpuLabel, sizeof(gpuLabel), "Utilization %%");
                    } else {
                        snprintf(gpuLabel, sizeof(gpuLabel), "GPU %d %%", g + 1);
                    }
                    bars[5 + g].setDimensions(barDims);
                    bars[5 + g].setValue(local.GPU_Util[g]);
                    bars[5 + g].draw({midX, startY + g * spacing}, gpuLabel, formatValue(local.GPU_Util[g]));
                }
            }
            else if (tile.config.title == "Network") {
                Rectangle tb = tile.titleBar();
                float contentTop = tb.y + tb.height + 6.0f;
                float contentBot = tile.bounds.y + tile.bounds.height - 8.0f;
                float contentH = contentBot - contentTop;
                float cw = tile.contentWidth();
                float midX = tile.bounds.x + tile.bounds.width / 2;

                barDims.maxSize = cw * 0.85f;
                bars[1].setDimensions(barDims);
                float barTotalH = bars[1].getTotalHeight();
                float availH = contentH;
                float sp = availH / 4.0f;
                if (sp < barTotalH + 4.0f) sp = barTotalH + 4.0f;
                if (sp > 55.0f) sp = 55.0f;
                float centerY = contentTop + barTotalH * 0.55f + sp * 1.5f;
                if (centerY + sp * 1.5f + barTotalH * 0.45f > contentBot - 2.0f) {
                    sp = (contentBot - 2.0f - contentTop - barTotalH) / 3.0f;
                    if (sp < barTotalH + 3.0f) sp = barTotalH + 3.0f;
                    centerY = contentTop + barTotalH * 0.55f + sp * 1.5f;
                }
                for (int n = 1; n <= 4; n++) bars[n].setDimensions(barDims);
                bars[1].draw({midX, centerY - sp * 1.5f}, "WiFi Up", formatValue(local.Wifi_Send));
                bars[2].draw({midX, centerY - sp * 0.5f}, "WiFi Down", formatValue(local.Wifi_Recv));
                bars[3].draw({midX, centerY + sp * 0.5f}, "Eth Up", formatValue(local.Ether_Send));
                bars[4].draw({midX, centerY + sp * 1.5f}, "Eth Down", formatValue(local.Ether_Recv));
            }
            else if (tile.config.title == "Storage") {
                const auto& diskList = stats1.GetDisks();
                int diskCount = static_cast<int>(diskList.size());
                int enabledCount = 0;
                for (int i = 0; i < diskCount; i++) {
                    if (diskList[i].enabled) enabledCount++;
                }
                Rectangle tb = tile.titleBar();
                float contentTop = tb.y + tb.height + 6.0f;
                float contentBot = tile.bounds.y + tile.bounds.height - 8.0f;
                float contentH = contentBot - contentTop;
                float cw = tile.contentWidth();
                float midX = tile.bounds.x + tile.bounds.width / 2;

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
            }
        }

        for (auto& t : tiles) {
            t.drawSnapPreview(activeTheme);
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

        if (showDiagnostics) drawDiagnosticsOverlay(sw, sh, fontSize);
        if (showSettings) drawSettingsOverlay(sw, sh, fontSize);

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
