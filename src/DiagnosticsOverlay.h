#ifndef DIAGNOSTICSOVERLAY_H
#define DIAGNOSTICSOVERLAY_H

#include "AppTheme.h"
#include "StatsData.h"

class StatsCollector;

void DrawDiagnosticsOverlay(int sw, int sh, int fontSize, const AppTheme& theme,
                            StatsData& statsData, StatsCollector& stats);

#endif
