#ifndef RENDERING_H
#define RENDERING_H

#include "raylib.h"
#include "StatsV1.h"
#include "BarV1.h"
#include "GaugeV1.h"
#include "ThemeV1.h"
#include "TileV1.h"
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

// ─── Design System ──────────────────────────────────────────

namespace DesignSystem {

inline constexpr float kTitleFontMax = 24.0f;
inline constexpr float kTitleFontMin = 10.0f;

inline float tileTitleFont(float tileW, float tileH) {
    float s = (std::min)(tileW * 0.08f, tileH * 0.09f);
    return (std::clamp)(s, kTitleFontMin, kTitleFontMax);
}

inline float modelFont(float tileW) {
    float f = (std::min)(tileW * 0.04f, 16.0f);
    return (std::max)(f, 8.0f);
}

inline BarV1::Theme makeBarTheme(const ThemeV1& t) {
    BarV1::Theme b;
    b.barBackgroundColor = t.barBackground;
    b.barForegroundColor = t.barForeground;
    b.textColor = t.textPrimary;
    return b;
}

inline GaugeV1::Theme makeGaugeTheme(const ThemeV1& t) {
    GaugeV1::Theme g;
    g.backgroundColor = t.windowBg;
    g.arcBackgroundColor = t.gaugeArcBg;
    g.arcActiveColor = t.gaugeArcActive;
    g.textColor = t.gaugeText;
    return g;
}

} // namespace DesignSystem

// ─── Cross-thread Stats Data ─────────────────────────────────

inline std::string formatValue(float value) {
    std::string s = std::to_string(value);
    size_t dot = s.find('.');
    return (dot != std::string::npos) ? s.substr(0, dot + 2) : s;
}

struct StatsData {
    std::atomic<float> CPU_Freq{0.0f};
    std::atomic<float> CPU_Util{0.0f};
    std::atomic<float> RAM_Util{0.0f};
    std::atomic<float> Wifi_Send{0.0f};
    std::atomic<float> Wifi_Recv{0.0f};
    std::atomic<float> Ether_Send{0.0f};
    std::atomic<float> Ether_Recv{0.0f};
    std::atomic<float> GPU_Util[4]{};
    std::atomic<float> GPU_VRAM[4]{};
    std::atomic<float> GPU_VRAMTotal[4]{};
    std::atomic<int>   GPU_Clock[4]{};
    std::atomic<int> GPUCount{0};
    std::atomic<int> DiskCount{0};
    std::atomic<float> DiskUtil[8]{};
};

// ─── Tile Content Rendering ──────────────────────────────────

inline void renderCPUTile(const TileV1& tile, const ThemeV1& theme, const StatsData& local,
                          StatsV1& stats, GaugeV1& gauge, BarV1& cpuBar) {
    Rectangle tb = tile.titleBar();
    float contentTop = tb.y + tb.height;
    float contentBot = tile.bounds.y + tile.bounds.height - 8.0f;
    float contentH = contentBot - contentTop;
    float cw = tile.contentWidth();
    float midX = tile.bounds.x + tile.bounds.width / 2;

    const char* cpuModel = stats.GETCPUModel();
    float mFont = DesignSystem::modelFont(tile.bounds.width);
    bool hasModel = cpuModel[0] != '\0';
    if (hasModel) {
        int mw = MeasureText(cpuModel, static_cast<int>(mFont));
        float maxW = tile.bounds.width - 16.0f;
        float useFont = mFont;
        while (useFont > 7.0f && mw > maxW) { useFont -= 0.5f; mw = MeasureText(cpuModel, static_cast<int>(useFont)); }
        DrawText(cpuModel, static_cast<int>(midX - mw / 2),
                 static_cast<int>(contentTop + 4.0f), static_cast<int>(useFont), theme.textSecondary);
    }
    float gaugeTop = contentTop + (hasModel ? mFont + 8.0f : 6.0f);
    float gaugeAvail = contentH * 0.60f;
    float gaugeSize = (std::min)(cw * 0.58f, gaugeAvail * 0.80f);
    if (gaugeSize < 40.0f) gaugeSize = 40.0f;
    gauge.setAutoScale(false);
    gauge.setBaseSize(gaugeSize);
    float gaugeCenterY = gaugeTop + gaugeSize * 0.52f;
    gauge.draw({midX, gaugeCenterY}, "Utilization");

    float gaugeBottom = gaugeCenterY + gaugeSize * 0.42f;
    BarV1::Dimensions barDims;
    barDims.maxSize = cw * 0.88f;
    cpuBar.setDimensions(barDims);
    float barTotalH = cpuBar.getTotalHeight();
    float barCenterY = contentBot - barTotalH * 0.45f;
    if (barCenterY < gaugeBottom + barTotalH * 0.5f + 6.0f) {
        barCenterY = gaugeBottom + barTotalH * 0.5f + 6.0f;
    }
    if (barCenterY + barTotalH * 0.5f > contentBot) barCenterY = contentBot - barTotalH * 0.5f - 2.0f;
    cpuBar.draw({midX, barCenterY}, "Frequency", formatValue(local.CPU_Freq.load()));
}

inline void renderRAMTile(const TileV1& tile, const ThemeV1& theme, const StatsData& local,
                          GaugeV1& gauge) {
    Rectangle tb = tile.titleBar();
    float contentTop = tb.y + tb.height;
    float contentBot = tile.bounds.y + tile.bounds.height - 8.0f;
    float contentH = contentBot - contentTop;
    float cw = tile.contentWidth();
    float midX = tile.bounds.x + tile.bounds.width / 2;

    float gaugeAvail = contentH * 0.80f;
    float gaugeSize = (std::min)(cw * 0.48f, gaugeAvail * 0.68f);
    if (gaugeSize < 30.0f) gaugeSize = 30.0f;
    gauge.setAutoScale(false);
    gauge.setBaseSize(gaugeSize);
    float gaugeCenterY = contentTop + gaugeAvail * 0.40f;
    gauge.draw({midX, gaugeCenterY}, "Load");
}

inline void renderGPUTile(const TileV1& tile, const ThemeV1& theme, const StatsData& local,
                          StatsV1& stats, std::vector<BarV1>& bars) {
    Rectangle tb = tile.titleBar();
    float contentTop = tb.y + tb.height;
    float contentBot = tile.bounds.y + tile.bounds.height - 8.0f;
    float contentH = contentBot - contentTop;
    float cw = tile.contentWidth();
    float midX = tile.bounds.x + tile.bounds.width / 2;

    const char* gpuModel = stats.GETGPUModel();
    float mFont = DesignSystem::modelFont(tile.bounds.width);
    bool hasModel = gpuModel[0] != '\0';
    if (hasModel) {
        int mw = MeasureText(gpuModel, static_cast<int>(mFont));
        float maxW = tile.bounds.width - 16.0f;
        float useFont = mFont;
        while (useFont > 7.0f && mw > maxW) { useFont -= 0.5f; mw = MeasureText(gpuModel, static_cast<int>(useFont)); }
        DrawText(gpuModel, static_cast<int>(midX - mw / 2),
                 static_cast<int>(contentTop + 4.0f), static_cast<int>(useFont), theme.textSecondary);
    }
    int gpuCount = local.GPUCount.load();
    if (gpuCount < 1) gpuCount = 1;
    float barsTop = contentTop + (hasModel ? mFont + 10.0f : 4.0f);
    float availH = contentBot - barsTop - 6.0f;

    int barsPerGpu = 3;
    int totalBars = gpuCount * barsPerGpu;
    int maxBars = (std::min)(totalBars, 12);

    BarV1::Dimensions barDims;
    barDims.maxSize = cw * 0.78f;
    bars[5].setDimensions(barDims);
    float barTotalH = bars[5].getTotalHeight();
    float spacing = availH / static_cast<float>(maxBars);
    if (spacing < barTotalH + 2.0f) spacing = barTotalH + 2.0f;
    if (spacing > 32.0f) spacing = 32.0f;

    float startY = barsTop + barTotalH * 0.55f;
    if (startY + (maxBars - 1) * spacing + barTotalH * 0.55f > contentBot - 2.0f) {
        spacing = (contentBot - 2.0f - startY - barTotalH * 0.55f) / (std::max)(1, maxBars - 1);
        if (spacing < barTotalH + 2.0f) spacing = barTotalH + 2.0f;
    }

    char label[64];
    int barIdx = 5;
    for (int g = 0; g < gpuCount && barIdx < 15; g++) {
        float utilVal = local.GPU_Util[g].load();
        float vramVal = local.GPU_VRAM[g].load();
        float vramTot = local.GPU_VRAMTotal[g].load();
        int clockVal = local.GPU_Clock[g].load();

        if (gpuCount == 1) snprintf(label, sizeof(label), "Util %%");
        else snprintf(label, sizeof(label), "GPU %d Util %%", g + 1);
        bars[barIdx].setDimensions(barDims);
        bars[barIdx].setValue(utilVal);
        bars[barIdx].draw({midX, startY + (barIdx - 5) * spacing}, label, formatValue(utilVal));
        barIdx++;

        if (vramTot > 0.0f) {
            float vramPct = (vramTot > 0.0f) ? (vramVal / vramTot) * 100.0f : 0.0f;
            if (gpuCount == 1) snprintf(label, sizeof(label), "VRAM %.1f/%.1f GB", vramVal, vramTot);
            else snprintf(label, sizeof(label), "GPU %d VRAM", g + 1);
            bars[barIdx].setDimensions(barDims);
            bars[barIdx].setValue(vramPct);
            bars[barIdx].draw({midX, startY + (barIdx - 5) * spacing}, label, formatValue(vramPct));
            barIdx++;
        }

        if (clockVal > 0 && barIdx < 15) {
            if (gpuCount == 1) snprintf(label, sizeof(label), "Clock %d MHz", clockVal);
            else snprintf(label, sizeof(label), "GPU %d %d MHz", g + 1, clockVal);
            bars[barIdx].setDimensions(barDims);
            bars[barIdx].setValue((std::min)(clockVal / 30.0f, 100.0f));
            bars[barIdx].draw({midX, startY + (barIdx - 5) * spacing}, label, std::to_string(clockVal) + " MHz");
            barIdx++;
        }
    }
}

inline void renderNetworkTile(const TileV1& tile, const ThemeV1& theme, const StatsData& local,
                              std::vector<BarV1>& bars) {
    Rectangle tb = tile.titleBar();
    float contentTop = tb.y + tb.height + 6.0f;
    float contentBot = tile.bounds.y + tile.bounds.height - 8.0f;
    float contentH = contentBot - contentTop;
    float cw = tile.contentWidth();
    float midX = tile.bounds.x + tile.bounds.width / 2;

    BarV1::Dimensions barDims;
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
    bars[1].draw({midX, centerY - sp * 1.5f}, "WiFi Up", formatValue(local.Wifi_Send.load()));
    bars[2].draw({midX, centerY - sp * 0.5f}, "WiFi Down", formatValue(local.Wifi_Recv.load()));
    bars[3].draw({midX, centerY + sp * 0.5f}, "Eth Up", formatValue(local.Ether_Send.load()));
    bars[4].draw({midX, centerY + sp * 1.5f}, "Eth Down", formatValue(local.Ether_Recv.load()));
}

inline void renderStorageTile(const TileV1& tile, const ThemeV1& theme, const StatsData& local,
                              StatsV1& stats, std::vector<BarV1>& bars, int fontSize) {
    const auto& diskList = stats.GetDisks();
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
                 static_cast<int>(contentTop + contentH * 0.45f), fontSize, theme.textSecondary);
    } else {
        int barIdx = 6;
        BarV1::Dimensions barDims;
        barDims.maxSize = cw * 0.78f;
        bars[6].setDimensions(barDims);
        float barTotalH = bars[6].getTotalHeight();
        float availH = contentH * 0.88f;
        float barH = availH / static_cast<float>(enabledCount);
        if (barH < barTotalH + 3.0f) barH = barTotalH + 3.0f;
        if (barH > 45.0f) barH = 45.0f;
        float startY = contentTop + barTotalH * 0.55f;
        if (startY + (enabledCount - 1) * barH + barTotalH * 0.55f > contentBot - 4.0f) {
            barH = (contentBot - 4.0f - startY - barTotalH * 0.55f) / (std::max)(1, enabledCount - 1);
            if (barH < barTotalH + 2.0f) barH = barTotalH + 2.0f;
        }
        for (int d = 0; d < diskCount && barIdx < 14; d++) {
            if (!diskList[d].enabled) continue;
            char buf[64];
            WideCharToMultiByte(CP_UTF8, 0, diskList[d].name.c_str(), -1, buf, sizeof(buf), nullptr, nullptr);
            bars[barIdx].setValue(local.DiskUtil[d].load());
            bars[barIdx].setDimensions(barDims);
            bars[barIdx].draw({midX, startY + (barIdx - 6) * barH}, buf, formatValue(local.DiskUtil[d].load()));
            barIdx++;
        }
    }
}

#endif
