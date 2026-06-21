#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include <windows.h>
#include "raylib.h"
#include "DiagnosticsOverlay.h"
#include "StatsCollector.h"
#include <cstdio>

void DrawDiagnosticsOverlay(int sw, int sh, int fontSize, const AppTheme& theme,
                            StatsData& statsData, StatsCollector& stats) {
    static float scrollOffset = 0.0f;
    int panelW = sw * 4 / 5, panelH = sh * 4 / 5;
    if (panelW < 420) panelW = 420;
    if (panelH < 320) panelH = 320;
    Rectangle panel = {static_cast<float>((sw - panelW) / 2), static_cast<float>((sh - panelH) / 2),
                       static_cast<float>(panelW), static_cast<float>(panelH)};

    Rectangle shadow = {panel.x + 4, panel.y + 6, panel.width, panel.height};
    DrawRectangleRounded(shadow, 0.08f, 16, {0, 0, 0, 80});
    DrawRectangleRounded(panel, 0.08f, 16, theme.panelOverlay);
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 1.0f, theme.tileBorder);

    int overlayFont = std::max(fontSize - 3, 10);
    int smallFont = std::max(overlayFont - 3, 10);
    int sectionFont = std::min(overlayFont + 2, 14);
    int x = static_cast<int>(panel.x + 20);
    int visibleTop = static_cast<int>(panel.y + 12);
    int visibleBot = static_cast<int>(panel.y + panelH - 12);

    DrawText("Diagnostics", x, visibleTop, fontSize, theme.titleText);

    int saveW = std::min(100, panelW - 200);
    if (saveW > 55) {
        Rectangle saveBtn = {panel.x + panelW - saveW - 14, panel.y + 10, static_cast<float>(saveW), 26};
        bool saveHover = CheckCollisionPointRec(GetMousePosition(), saveBtn);
        Color sbBg = saveHover ? theme.accentColor : theme.toggleInactive;
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
            DrawText(title, static_cast<int>(secX), y, sectionFont, theme.accentColor);
            float lineW = std::min(panelW - 100.0f, 400.0f);
            DrawLineEx({secX + MeasureText(title, sectionFont) + 10, y + sectionFont / 2.0f},
                       {secX + lineW, y + sectionFont / 2.0f},
                       1.0f, theme.lineColor);
        }
        y += sectionFont + 8;
    };

    auto drawLine = [&](const char* label, const char* value) {
        if (y > visibleTop - 20 && y < visibleBot + 20) {
            float lx = panel.x + 28;
            DrawText(label, static_cast<int>(lx), y, smallFont, theme.textPrimary);
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
                DrawText(value, static_cast<int>(vx), y, smallerVFont, theme.textSecondary);
            } else {
                DrawText(value, static_cast<int>(vx), y, smallFont, theme.textSecondary);
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
        snprintf(buf, sizeof(buf), "%.1f %%", statsData.CPU_Util.load());
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
                char lineBuf[256];
                const char* displayName = stats.GETGPUDisplayName(i);
                if (displayName[0] != '\0') {
                    snprintf(lineBuf, sizeof(lineBuf), "%.1f %%", stats.GETGPUUtilization(i));
                    drawLine(displayName, lineBuf);
                } else {
                    char wbuf[128];
                    WideCharToMultiByte(CP_UTF8, 0, stats.GETGPUName(i), -1, wbuf, sizeof(wbuf), nullptr, nullptr);
                    snprintf(lineBuf, sizeof(lineBuf), "%.1f %%", stats.GETGPUUtilization(i));
                    drawLine(wbuf, lineBuf);
                }
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
        DrawRectangleRounded(sb, 0.5f, 6, theme.scrollbarColor);
    }
    EndScissorMode();
}
