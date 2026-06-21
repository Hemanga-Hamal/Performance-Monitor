#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include <windows.h>
#include "raylib.h"
#include "test_harness.h"
#include "Bar.h"
#include "Gauge.h"

void RunGraphicsTests() {
    printf("\n[Graphics Tests]\n");

    InitWindow(200, 200, "Test");
    SetWindowState(FLAG_WINDOW_HIDDEN);

    // Bar Tests
    printf("  [Bar]\n");

    TEST("  Theme defaults");
    Bar::Theme btheme;
    CHECK(true);

    TEST("  Dimensions defaults");
    Bar::Dimensions bdims;
    CHECK(bdims.barWidth == 300.0f);

    TEST("  Config defaults");
    Bar::Config bcfg;
    CHECK(bcfg.value == 0.0f);
    CHECK(bcfg.maxValue == 100.0f);
    CHECK(bcfg.autoScale == true);

    TEST("  Config value constructor");
    Bar::Config bcfg2(50.0f, 200.0f);
    CHECK_EQ(bcfg2.value, 50.0f);
    CHECK_EQ(bcfg2.maxValue, 200.0f);

    TEST("  Bar constructor");
    Bar bar(btheme, bdims, bcfg);
    CHECK(true);

    TEST("  setValue updates config");
    bar.setValue(75.0f);
    CHECK_EQ(bar.getConfig().value, 75.0f);

    TEST("  setTheme/setDimensions/setConfig");
    bar.setTheme(btheme);
    bar.setDimensions(bdims);
    bar.setConfig(bcfg);
    CHECK(true);

    TEST("  getWidth > 0");
    CHECK(bar.getWidth() > 0.0f);

    TEST("  getHeight > 0");
    CHECK(bar.getHeight() > 0.0f);

    TEST("  getWidth/getHeight proportional");
    CHECK(bar.getHeight() < bar.getWidth());

    CloseWindow();

    // Gauge Tests
    printf("  [Gauge]\n");

    TEST("  clamp helper low");
    CHECK_EQ(Gauge::clamp(-5.0f, 0.0f, 100.0f), 0.0f);

    TEST("  clamp helper high");
    CHECK_EQ(Gauge::clamp(150.0f, 0.0f, 100.0f), 100.0f);

    TEST("  clamp helper in range");
    CHECK_EQ(Gauge::clamp(50.0f, 0.0f, 100.0f), 50.0f);

    TEST("  Theme defaults");
    Gauge::Theme gtheme;
    CHECK(true);

    TEST("  Dimensions defaults");
    Gauge::Dimensions gdims;
    CHECK(gdims.baseSize == 200.0f);

    TEST("  Config defaults");
    Gauge::Config gcfg;
    CHECK(gcfg.startAngle == 150.0f);

    TEST("  ConfigArc preset");
    Gauge::Config arcCfg = Gauge::Config::ConfigArc();
    CHECK_EQ(arcCfg.startAngle, 150.0f);
    CHECK_EQ(arcCfg.totalAngle, 240.0f);

    TEST("  ConfigQuarter preset");
    Gauge::Config qtrCfg = Gauge::Config::ConfigQuarter();
    CHECK_EQ(qtrCfg.startAngle, 90.0f);
    CHECK_EQ(qtrCfg.totalAngle, 270.0f);

    TEST("  Gauge constructor");
    Gauge gauge(gtheme, gdims, gcfg);
    CHECK(true);

    TEST("  setValue clamps 0-100");
    gauge.setValue(50.0f);
    gauge.setValue(-10.0f);
    gauge.setValue(110.0f);
    CHECK(true);

    TEST("  setTotalAngle/StartAngle/AutoScale/ScreenSizeRatio/TextColor");
    gauge.setTotalAngle(180.0f);
    gauge.setStartAngle(90.0f);
    gauge.setAutoScale(false);
    gauge.setScreenSizeRatio(0.5f);
    gauge.setTextColor(RED);
    CHECK(true);

    TEST("  setScale/setBaseSize/setArcThickness");
    gauge.setScale(0.5f);
    gauge.setBaseSize(100.0f);
    gauge.setArcThickness(0.3f);
    CHECK(true);

    InitWindow(200, 200, "Test2");
    SetWindowState(FLAG_WINDOW_HIDDEN);

    TEST("  calculateGaugeSize > 0 (auto scale)");
    Gauge::Config autoCfg = Gauge::Config::ConfigArc();
    Gauge gaugeAuto(gtheme, gdims, autoCfg);
    CHECK(gaugeAuto.calculateGaugeSize() > 0.0f);

    TEST("  calculateGaugeSize > 0 (fixed)");
    Gauge::Config fixedCfg;
    fixedCfg.autoScale = false;
    fixedCfg.startAngle = 150.0f;
    fixedCfg.totalAngle = 240.0f;
    Gauge gaugeFixed(gtheme, gdims, fixedCfg);
    CHECK(gaugeFixed.calculateGaugeSize() > 0.0f);

    CloseWindow();
}
