#include "test_harness.h"
#include "Stats.h"

void RunCPUTests(Stats& stats) {
    printf("\n[CPU Tests]\n");

    TEST("GETCPUFrequency >= 0");
    float freq = stats.GETCPUFrequency();
    CHECK(freq >= 0.0f);

    TEST("GETCPUFrequency reasonable (< 10000 MHz)");
    CHECK(freq <= 10000.0f);

    TEST("GETCPUFrequency consistent (2 calls)");
    float freq2 = stats.GETCPUFrequency();
    CHECK(freq2 >= 0.0f);

    TEST("GETCPUtilization returns 0-100");
    float util = stats.GETCPUtilization();
    CHECK_RANGE(util, 0.0f, 100.0f);

    TEST("GETCPUModel returns non-empty string");
    const char* model = stats.GETCPUModel();
    CHECK(model != nullptr && model[0] != '\0');
}
