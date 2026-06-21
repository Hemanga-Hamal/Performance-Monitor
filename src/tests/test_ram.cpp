#include "test_harness.h"
#include "Stats.h"

void RunRAMTests(Stats& stats) {
    printf("\n[RAM Tests]\n");

    TEST("GETRAMTotal > 0");
    float ramTotal = stats.GETRAMTotal();
    CHECK(ramTotal > 0.0f);

    TEST("GETRAMUsed >= 0");
    float ramUsed = stats.GETRAMUsed();
    CHECK(ramUsed >= 0.0f);

    TEST("GETRAMUsed <= GETRAMTotal");
    CHECK(ramUsed <= ramTotal + 1.0f);

    TEST("GETRAMUtilization returns 0-100");
    float ramUtil = stats.GETRAMUtilization();
    CHECK_RANGE(ramUtil, 0.0f, 100.0f);
}
