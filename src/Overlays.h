#ifndef OVERLAYS_H
#define OVERLAYS_H

#include "raylib.h"
#include "StatsV1.h"
#include "ThemeV1.h"
#include <cstdio>
#include <string>
#include <algorithm>

inline void drawDiagnosticsOverlay(const ThemeV1& activeTheme, int sw, int sh, int fontSize, StatsV1& stats) {
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

    int smallFont = (std::max)(fontSize - 3, 10);
    int sectionFont = (std::max)(fontSize - 1, 12);
    int x = static_cast<int>(panel.x + 20);
    int visibleTop = static_cast<int>(panel.y + 12);
    int visibleBot = static_cast<int>(panel.y + panelH - 12);

    DrawText("Diagnostics", x, visibleTop, fontSize, activeTheme.titleText);

    int saveW = (std::min)(100, panelW - 200);
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
                fwprintf(f, L"  Model: %hs\n", stats.GETCPUModel());
                fwprintf(f, L"  Frequency: %.0f MHz\n", stats.GETCPUFrequency());
                fwprintf(f, L"  Utilization: %.1f %%\n", stats.GETCPUtilization());
                fwprintf(f, L"\n[RAM]\n");
                fwprintf(f, L"  Total: %.1f GB\n", stats.GETRAMTotal());
                fwprintf(f, L"  Used: %.1f GB\n", stats.GETRAMUsed());
                fwprintf(f, L"  Utilization: %.1f %%\n", stats.GETRAMUtilization());
                fwprintf(f, L"\n[GPU]\n");
                fwprintf(f, L"  Model: %hs\n", stats.GETGPUModel());
                int gc = stats.GETGPUCount();
                for (int i = 0; i < gc; i++) {
                    char nbuf[128];
                    WideCharToMultiByte(CP_UTF8, 0, stats.GETGPUName(i), -1, nbuf, sizeof(nbuf), nullptr, nullptr);
                    fwprintf(f, L"  %hs: %.1f %%\n", nbuf, stats.GETGPUUtilization(i));
                }
                fwprintf(f, L"\n[Disks]\n");
                for (int i = 0; i < stats.GETDiskCount(); i++) {
                    char nbuf[64];
                    WideCharToMultiByte(CP_UTF8, 0, stats.GETDiskName(i), -1, nbuf, sizeof(nbuf), nullptr, nullptr);
                    fwprintf(f, L"  %hs: %.1f GB / %.1f GB (%.1f %%)\n",
                             nbuf, stats.GETDiskUsed(i), stats.GETDiskTotal(i), stats.GETDiskUtilization(i));
                }
                fwprintf(f, L"\n[Network]\n");
                const auto& adps = stats.GetAdapters();
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
            float lineW = (std::min)(panelW - 100.0f, 400.0f);
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
        snprintf(buf, sizeof(buf), "%s", stats.GETCPUModel());
        drawLine("Model:", buf);
        snprintf(buf, sizeof(buf), "%.0f MHz", stats.GETCPUFrequency());
        drawLine("Frequency:", buf);
        snprintf(buf, sizeof(buf), "%.1f %%", stats.GETCPUtilization());
        drawLine("Utilization:", buf);
    }
    y += 6;

    drawSection("RAM");
    {
        char buf[128];
        snprintf(buf, sizeof(buf), "%.1f GB", stats.GETRAMTotal());
        drawLine("Total:", buf);
        snprintf(buf, sizeof(buf), "%.1f GB", stats.GETRAMUsed());
        drawLine("Used:", buf);
        snprintf(buf, sizeof(buf), "%.1f %%", stats.GETRAMUtilization());
        drawLine("Utilization:", buf);
    }
    y += 6;

    drawSection("GPU");
    {
        char buf[256];
        if (stats.IsGPUAvailable()) {
            snprintf(buf, sizeof(buf), "%s", stats.GETGPUModel());
            drawLine("Model:", buf);
            int gpuCount = stats.GETGPUCount();
            for (int i = 0; i < gpuCount; i++) {
                char wbuf[128], lineBuf[256];
                WideCharToMultiByte(CP_UTF8, 0, stats.GETGPUName(i), -1, wbuf, sizeof(wbuf), nullptr, nullptr);
                snprintf(lineBuf, sizeof(lineBuf), "%.1f %%", stats.GETGPUUtilization(i));
                drawLine(wbuf, lineBuf);
            }
        } else {
            drawLine("Status:", "Not detected");
        }
    }
    y += 6;

    drawSection("Disks");
    for (int i = 0; i < stats.GETDiskCount(); i++) {
        if (y > visibleBot) break;
        char nameBuf[64], lineBuf[256];
        WideCharToMultiByte(CP_UTF8, 0, stats.GETDiskName(i), -1, nameBuf, sizeof(nameBuf), nullptr, nullptr);
        snprintf(lineBuf, sizeof(lineBuf), "%.1f GB total, %.1f GB used, %.1f %%",
                 stats.GETDiskTotal(i), stats.GETDiskUsed(i), stats.GETDiskUtilization(i));
        drawLine(nameBuf, lineBuf);
    }
    y += 6;

    drawSection("Network");
    const auto& adapters = stats.GetDiscoveredAdapters();
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

inline void drawSettingsOverlay(const ThemeV1& activeTheme, int sw, int sh, int fontSize, StatsV1& stats,
                                bool tileEnabled[5]) {
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

    int smallFont = (std::max)(fontSize - 2, 10);
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
        int dc = stats.GETDiskCount();
        snprintf(diskInfo, sizeof(diskInfo), "%d drive(s)", dc);
        if (drawToggle("Storage Tile", diskInfo, tileEnabled[4], y)) tileEnabled[4] = !tileEnabled[4];
    }

    y += 10;

    const auto& disks = stats.GetDisks();
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
                stats.SetDiskEnabled(static_cast<int>(i), !enabled);
            }
        }
        y += 10;
    }

    const auto& adapters = stats.GetAdapters();
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
                stats.SetAdapterEnabled(static_cast<int>(i), !enabled);
            }
        }
    }

    EndScissorMode();
}

#endif
