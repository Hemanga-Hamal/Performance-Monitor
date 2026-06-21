#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "test_harness.h"
#include "StatsCollector.h"

int testPassed = 0;
int testFailed = 0;

int main() {
    printf("\n=== Performance Monitor Tests ===\n");

    StatsCollector stats;

    RunCPUTests(stats);
    RunRAMTests(stats);
    RunDiskTests(stats);
    RunNetworkTests(stats);
    RunGPUTests(stats);
    RunGraphicsTests();

    printf("\n=============================\n");
    printf("  Passed: %d\n", testPassed);
    printf("  Failed: %d\n", testFailed);
    printf("=============================\n");

    return testFailed > 0 ? 1 : 0;
}
