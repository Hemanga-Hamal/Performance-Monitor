#include "test_harness.h"
#include "StatsV1.h"

void RunGPUTests(StatsV1& stats) {
    printf("\n[GPU Tests]\n");

    TEST("IsGPUAvailable returns bool");
    bool avail = stats.IsGPUAvailable();
    CHECK(avail == true || avail == false);

    TEST("GETGPUUtilization returns 0-100 (if available)");
    float util = stats.GETGPUUtilization();
    CHECK_RANGE(util, 0.0f, 100.0f);

    TEST("GETGPUModel returns string");
    const char* model = stats.GETGPUModel();
    CHECK(model != nullptr);

    TEST("GETGPUName returns non-empty if available");
    const wchar_t* name = stats.GETGPUName();
    if (avail) {
        CHECK(name != nullptr && name[0] != L'\0');
    } else {
        CHECK(name != nullptr);
    }
}
