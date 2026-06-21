#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include <cstdio>

extern int testPassed;
extern int testFailed;

#define TEST(name) do { printf("  %-55s ", name); } while(0)
#define CHECK(cond) do { \
    if (cond) { printf("PASS\n"); testPassed++; } \
    else { printf("FAIL  (line %d)\n", __LINE__); testFailed++; } \
} while(0)
#define CHECK_EQ(a, b) do { \
    if ((a) == (b)) { printf("PASS\n"); testPassed++; } \
    else { printf("FAIL  (line %d: %d vs %d)\n", __LINE__, (int)(a), (int)(b)); testFailed++; } \
} while(0)
#define CHECK_RANGE(val, lo, hi) do { \
    if ((val) >= (lo) && (val) <= (hi)) { printf("PASS\n"); testPassed++; } \
    else { printf("FAIL  (line %d: %.1f not in [%.1f,%.1f])\n", __LINE__, (float)(val), (float)(lo), (float)(hi)); testFailed++; } \
} while(0)

void RunCPUTests(class Stats& stats);
void RunRAMTests(class Stats& stats);
void RunDiskTests(class Stats& stats);
void RunNetworkTests(class Stats& stats);
void RunGPUTests(class Stats& stats);
void RunGraphicsTests();

#endif
