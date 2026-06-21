#include "test_harness.h"
#include "StatsCollector.h"

void RunNetworkTests(StatsCollector& stats) {
    printf("\n[Network Tests]\n");

    TEST("GETWiFiSend >= 0");
    CHECK(stats.GETWiFiSend() >= 0.0f);

    TEST("GETWiFiReceive >= 0");
    CHECK(stats.GETWiFiReceive() >= 0.0f);

    TEST("GETEthernetSend >= 0");
    CHECK(stats.GETEthernetSend() >= 0.0f);

    TEST("GETEthernetReceive >= 0");
    CHECK(stats.GETEthernetReceive() >= 0.0f);

    TEST("GetDiscoveredAdapters not empty (or handled)");
    const auto& adapters = stats.GetDiscoveredAdapters();
    CHECK(adapters.size() >= 0);
}
