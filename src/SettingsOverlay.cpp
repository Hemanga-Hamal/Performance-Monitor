#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include <windows.h>
#include "raylib.h"
#include "SettingsOverlay.h"
#include <cstdio>

void DrawSettingsOverlay(int sw, int sh, int fontSize, const AppTheme& theme,
                         bool tileEnabled[], StatsCollector& stats) {
    bool configChanged = false;
    static float settingsScroll = 0.0f;
    int panelW = 370, panelH = sh * 4 / 5;
    if (panelH < 320) panelH = 320;
    Rectangle panel = {static_cast<float>(sw - panelW - 20), 50.0f,
                       static_cast<float>(panelW), static_cast<float>(panelH)};

    Rectangle shadow = {panel.x + 3, panel.y + 5, panel.width, panel.height};
    DrawRectangleRounded(shadow, 0.08f, 16, {0, 0, 0, 70});
    DrawRectangleRounded(panel, 0.08f, 16, theme.panelOverlay);
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 1.0f, theme.tileBorder);
    DrawText("Settings", static_cast<int>(panel.x + 20), static_cast<int>(panel.y + 14), fontSize, theme.titleText);

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
            Color bg = enabled ? theme.toggleActive : theme.toggleInactive;
            DrawRectangleRounded(cb, 0.25f, 8, bg);
            DrawRectangleRoundedLinesEx(cb, 0.25f, 8, 1.0f, theme.tileBorder);
            if (enabled) {
                const char* check = "\xE2\x9C\x93";
                int cw = MeasureText(check, smallFont);
                DrawText(check, static_cast<int>(cb.x + (20 - cw) / 2), y + 2, smallFont, WHITE);
            }
            float lx = cbX + 30;
            float maxTextW = panel.x + panelW - lx - 20.0f;
            DrawText(label, static_cast<int>(lx), y, smallFont, theme.textPrimary);
            if (info[0]) {
                int infoFont = smallFont - 3;
                if (infoFont < 8) infoFont = 8;
                int iw = MeasureText(info, infoFont);
                if (iw > maxTextW && maxTextW > 20.0f) {
                    while (infoFont > 8) { infoFont--; iw = MeasureText(info, infoFont); if (iw <= maxTextW) break; }
                }
                DrawText(info, static_cast<int>(lx), y + smallFont + 3, infoFont, theme.textSecondary);
            }
        }
        y += smallFont * 2 + 14;
        return CheckCollisionPointRec(GetMousePosition(), cb) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    };

    int y = yBase;

    {
        char cpuInfo[128] = "";
        snprintf(cpuInfo, sizeof(cpuInfo), "%s", stats.GETCPUModel());
        if (drawToggle("CPU Tile", cpuInfo, tileEnabled[0], y)) tileEnabled[0] = !tileEnabled[0];

        char ramInfo[64] = "";
        snprintf(ramInfo, sizeof(ramInfo), "%.1f GB total", stats.GETRAMTotal());
        if (drawToggle("RAM Tile", ramInfo, tileEnabled[1], y)) tileEnabled[1] = !tileEnabled[1];

        char gpuInfo[128] = "";
        if (stats.IsGPUAvailable()) {
            int gpuCount = stats.GETGPUCount();
            if (gpuCount > 1) {
                snprintf(gpuInfo, sizeof(gpuInfo), "%s (%d GPUs)", stats.GETGPUModel(), gpuCount);
            } else {
                char gbuf[64];
                WideCharToMultiByte(CP_UTF8, 0, stats.GETGPUName(0), -1, gbuf, sizeof(gbuf), nullptr, nullptr);
                snprintf(gpuInfo, sizeof(gpuInfo), "%s (%s)", stats.GETGPUModel(), gbuf);
            }
        } else {
            snprintf(gpuInfo, sizeof(gpuInfo), "Not detected");
        }
        if (drawToggle("GPU Tile", gpuInfo, tileEnabled[2], y)) tileEnabled[2] = !tileEnabled[2];

        if (drawToggle("Network Tile", "WiFi + Ethernet", tileEnabled[3], y)) tileEnabled[3] = !tileEnabled[3];

        char diskInfo[64] = "";
        int dc = stats.GETEnabledDiskCount();
        snprintf(diskInfo, sizeof(diskInfo), "%d drive(s)", dc);
        if (drawToggle("Storage Tile", diskInfo, tileEnabled[4], y)) tileEnabled[4] = !tileEnabled[4];
    }

    y += 10;

    const auto& disks = stats.GetDisks();
    if (!disks.empty()) {
        float secX = panel.x + 18;
        if (y + smallFont + 8 > visibleTop && y < visibleBot)
            DrawText("Disks", static_cast<int>(secX), y, smallFont, theme.accentColor);
        y += smallFont + 8;
        for (size_t i = 0; i < disks.size(); i++) {
            char label[64], info[128];
            WideCharToMultiByte(CP_UTF8, 0, disks[i].name.c_str(), -1, label, sizeof(label), nullptr, nullptr);
            snprintf(info, sizeof(info), "%.1f GB total", disks[i].totalGB);
            bool enabled = disks[i].enabled;
            if (drawToggle(label, info, enabled, y)) {
                stats.SetDiskEnabled(static_cast<int>(i), !enabled);
            }
        }
        y += 10;
    }

    const auto& adapters = stats.GetAdapters();
    if (!adapters.empty()) {
        float secX = panel.x + 18;
        if (y + smallFont + 8 > visibleTop && y < visibleBot)
            DrawText("Network", static_cast<int>(secX), y, smallFont, theme.accentColor);
        y += smallFont + 8;
        for (size_t i = 0; i < adapters.size(); i++) {
            char label[128], info[64];
            WideCharToMultiByte(CP_UTF8, 0, adapters[i].name.c_str(), -1, label, sizeof(label), nullptr, nullptr);
            snprintf(info, sizeof(info), "%s", adapters[i].isWiFi ? "WiFi" : (adapters[i].isEthernet ? "Ethernet" : "Other"));
            bool enabled = adapters[i].enabled;
            if (drawToggle(label, info, enabled, y)) {
                stats.SetAdapterEnabled(static_cast<int>(i), !enabled);
            }
        }
    }

    EndScissorMode();
}
