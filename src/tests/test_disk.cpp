#include "test_harness.h"
#include "Stats.h"

void RunDiskTests(Stats& stats) {
    printf("\n[Disk Tests]\n");

    TEST("GETDiskCount > 0");
    int diskCount = stats.GETDiskCount();
    CHECK(diskCount > 0);

    for (int i = 0; i < diskCount; i++) {
        printf("  [Disk %d]\n", i);

        TEST("    GETDiskName not empty");
        const wchar_t* name = stats.GETDiskName(i);
        CHECK(name != nullptr && name[0] != L'\0');

        TEST("    GETDiskTotal > 0");
        float total = stats.GETDiskTotal(i);
        CHECK(total > 0.0f);

        TEST("    GETDiskUsed >= 0");
        float used = stats.GETDiskUsed(i);
        CHECK(used >= 0.0f);

        TEST("    GETDiskUsed <= GETDiskTotal");
        CHECK(used <= total + 1.0f);

        TEST("    GETDiskUtilization returns 0-100");
        float diskUtil = stats.GETDiskUtilization(i);
        CHECK_RANGE(diskUtil, 0.0f, 100.0f);
    }
}
