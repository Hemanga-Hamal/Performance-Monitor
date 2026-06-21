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
    std::atomic<float> RAM_Total{0.0f};
    std::atomic<float> RAM_Used{0.0f};
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
    std::string cpuModel;
    std::string gpuModel;
};

// ─── Tile Content Rendering ──────────────────────────────────

inline void renderCPUTile(const TileV1& tile, const ThemeV1& theme, const StatsData& local,
                          StatsV1& stats, GaugeV1& gauge, BarV1& cpuBar) {
    Rectangle tb = tile.titleBar();
    float contentTop = tb.y + tb.height + 8.0f;
    float contentBot = tile.bounds.y + tile.bounds.height * 0.90f;
    float contentH = contentBot - contentTop;
    float contentW = tile.bounds.width * 0.90f;
    float midX = tile.bounds.x + tile.bounds.width / 2;

    BeginScissorMode(static_cast<int>(tile.bounds.x), static_cast<int>(tile.bounds.y),
                     static_cast<int>(tile.bounds.width), static_cast<int>(tile.bounds.height));

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
    float mainTop = contentTop + (hasModel ? mFont + 12.0f : 4.0f);
    float avail = contentBot - mainTop;
    float half = avail * 0.50f;

    float gaugeSize = (std::min)(contentW, half * 0.90f);
    if (gaugeSize < 30.0f) gaugeSize = 30.0f;
    gauge.setAutoScale(false);
    gauge.setBaseSize(gaugeSize);
    float gaugeCenterY = mainTop + half * 0.50f;
    gauge.draw({midX, gaugeCenterY}, "Utilization");

    BarV1::Dimensions barDims;
    barDims.barWidth = contentW;
    barDims.maxSize = contentW;
    barDims.minSize = (std::min)(60.0f, contentW * 0.45f);
    cpuBar.setDimensions(barDims);
    float barCenterY = mainTop + half * 1.50f;
    cpuBar.draw({midX, barCenterY}, "Frequency", formatValue(local.CPU_Freq.load()));

    EndScissorMode();
}

inline void renderRAMTile(const TileV1& tile, const ThemeV1& theme, const StatsData& local,
                          GaugeV1& gauge, BarV1& ramBar) {
    Rectangle tb = tile.titleBar();
    float contentTop = tb.y + tb.height + 8.0f;
    float contentBot = tile.bounds.y + tile.bounds.height * 0.90f;
    float contentH = contentBot - contentTop;
    float contentW = tile.bounds.width * 0.90f;
    float midX = tile.bounds.x + tile.bounds.width / 2;

    BeginScissorMode(static_cast<int>(tile.bounds.x), static_cast<int>(tile.bounds.y),
                     static_cast<int>(tile.bounds.width), static_cast<int>(tile.bounds.height));

    float half = contentH * 0.50f;

    float gaugeSize = (std::min)(contentW, half * 0.90f);
    if (gaugeSize < 30.0f) gaugeSize = 30.0f;
    gauge.setAutoScale(false);
    gauge.setBaseSize(gaugeSize);
    float gaugeCenterY = contentTop + half * 0.50f;
    gauge.draw({midX, gaugeCenterY}, "Load");

    BarV1::Dimensions barDims;
    barDims.barWidth = contentW;
    barDims.maxSize = contentW;
    barDims.minSize = (std::min)(60.0f, contentW * 0.45f);
    ramBar.setDimensions(barDims);
    float barCenterY = contentTop + half * 1.50f;
    float used = local.RAM_Used.load();
    float total = local.RAM_Total.load();
    char label[64];
    if (total > 0.0f) {
        snprintf(label, sizeof(label), "%.1f / %.1f GB", used, total);
    } else {
        snprintf(label, sizeof(label), "%.1f GB", used);
    }
    ramBar.draw({midX, barCenterY}, label, formatValue(local.RAM_Util.load()));

    EndScissorMode();
}

inline void renderGPUTile(const TileV1& tile, const ThemeV1& theme, const StatsData& local,
                          StatsV1& stats, std::vector<BarV1>& bars, GaugeV1& gaugeGPU) {
    Rectangle tb = tile.titleBar();
    float contentTop = tb.y + tb.height + 8.0f;
    float contentBot = tile.bounds.y + tile.bounds.height * 0.90f;
    float contentW = tile.bounds.width * 0.90f;
    float midX = tile.bounds.x + tile.bounds.width / 2;

    BeginScissorMode(static_cast<int>(tile.bounds.x), static_cast<int>(tile.bounds.y),
                     static_cast<int>(tile.bounds.width), static_cast<int>(tile.bounds.height));

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
    float mainTop = contentTop + (hasModel ? mFont + 12.0f : 4.0f);
    float contentH = contentBot - mainTop;

    float vramUsed = local.GPU_VRAM[0].load();
    float vramTot = local.GPU_VRAMTotal[0].load();
    int clockVal = local.GPU_Clock[0].load();
    bool hasVram = vramTot > 0.0f;
    bool hasClock = clockVal > 0;
    int barCount = (hasVram ? 1 : 0) + (hasClock ? 1 : 0);

    float gaugeFrac = 1.0f / (1.0f + barCount * 0.50f);
    float gaugeSectionH = contentH * gaugeFrac;
    float barsAvail = contentH - gaugeSectionH;
    if (barsAvail < 4.0f) barsAvail = 4.0f;

    float gaugeSize = (std::min)(contentW, gaugeSectionH * 0.90f);
    if (gaugeSize < 40.0f) gaugeSize = 40.0f;
    gaugeGPU.setAutoScale(false);
    gaugeGPU.setBaseSize(gaugeSize);
    float gaugeCenterY = mainTop + gaugeSectionH * 0.50f;
    gaugeGPU.draw({midX, gaugeCenterY}, "Utilization");

    float barsTop = mainTop + gaugeSectionH + 8.0f;
    float sectionH = barCount > 0 ? barsAvail / static_cast<float>(barCount) : barsAvail;

    BarV1::Dimensions barDims;
    barDims.barWidth = contentW;
    barDims.maxSize = contentW;
    barDims.minSize = (std::min)(60.0f, contentW * 0.45f);

    int bi = 0;
    char label[64];
    if (hasVram) {
        float vramPct = (vramUsed / vramTot) * 100.0f;
        snprintf(label, sizeof(label), "VRAM %.1f/%.1f GB", vramUsed, vramTot);
        bars[5].setDimensions(barDims);
        bars[5].setValue(vramPct);
        bars[5].draw({midX, barsTop + sectionH * (bi + 0.5f)}, label, formatValue(vramPct));
        bi++;
    }
    if (hasClock) {
        snprintf(label, sizeof(label), "Clock %d MHz", clockVal);
        bars[6].setDimensions(barDims);
        bars[6].setValue((std::min)(clockVal / 30.0f, 100.0f));
        bars[6].draw({midX, barsTop + sectionH * (bi + 0.5f)}, label, std::to_string(clockVal) + " MHz");
        bi++;
    }

    EndScissorMode();
}

inline void renderNetworkTile(const TileV1& tile, const ThemeV1& theme, const StatsData& local,
                              std::vector<BarV1>& bars) {
    Rectangle tb = tile.titleBar();
    float contentTop = tb.y + tb.height + 8.0f;
    float contentBot = tile.bounds.y + tile.bounds.height * 0.90f;
    float contentH = contentBot - contentTop;
    float contentW = tile.bounds.width * 0.90f;
    float midX = tile.bounds.x + tile.bounds.width / 2;

    BeginScissorMode(static_cast<int>(tile.bounds.x), static_cast<int>(tile.bounds.y),
                     static_cast<int>(tile.bounds.width), static_cast<int>(tile.bounds.height));

    BarV1::Dimensions barDims;
    barDims.barWidth = contentW;
    barDims.maxSize = contentW;
    barDims.minSize = (std::min)(60.0f, contentW * 0.45f);
    for (int n = 1; n <= 4; n++) bars[n].setDimensions(barDims);
    float sectionH = contentH / 4.0f;

    bars[1].draw({midX, contentTop + sectionH * 0.5f}, "WiFi Up",   formatValue(local.Wifi_Send.load()));
    bars[2].draw({midX, contentTop + sectionH * 1.5f}, "WiFi Down", formatValue(local.Wifi_Recv.load()));
    bars[3].draw({midX, contentTop + sectionH * 2.5f}, "Eth Up",    formatValue(local.Ether_Send.load()));
    bars[4].draw({midX, contentTop + sectionH * 3.5f}, "Eth Down",  formatValue(local.Ether_Recv.load()));

    EndScissorMode();
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
    float contentTop = tb.y + tb.height + 8.0f;
    float contentBot = tile.bounds.y + tile.bounds.height * 0.90f;
    float contentH = contentBot - contentTop;
    float contentW = tile.bounds.width * 0.90f;
    float midX = tile.bounds.x + tile.bounds.width / 2;

    BeginScissorMode(static_cast<int>(tile.bounds.x), static_cast<int>(tile.bounds.y),
                     static_cast<int>(tile.bounds.width), static_cast<int>(tile.bounds.height));

    if (enabledCount == 0) {
        DrawText("No drives enabled", static_cast<int>(midX - MeasureText("No drives enabled", fontSize) / 2),
                 static_cast<int>(contentTop + contentH * 0.50f - fontSize * 0.5f), fontSize, theme.textSecondary);
    } else {
        int barIdx = 6;
        BarV1::Dimensions barDims;
        barDims.barWidth = contentW;
        barDims.maxSize = contentW;
        barDims.minSize = (std::min)(60.0f, contentW * 0.45f);
        bars[6].setDimensions(barDims);
        float sectionH = contentH / static_cast<float>(enabledCount);
        for (int d = 0, i = 0; d < diskCount && barIdx < 14; d++) {
            if (!diskList[d].enabled) continue;
            char buf[64];
            WideCharToMultiByte(CP_UTF8, 0, diskList[d].name.c_str(), -1, buf, sizeof(buf), nullptr, nullptr);
            bars[barIdx].setValue(local.DiskUtil[d].load());
            bars[barIdx].setDimensions(barDims);
            bars[barIdx].draw({midX, contentTop + sectionH * (i + 0.5f)}, buf, formatValue(local.DiskUtil[d].load()));
            barIdx++; i++;
        }
    }

    EndScissorMode();
}

#endif
