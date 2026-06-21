#ifndef SETTINGSOVERLAY_H
#define SETTINGSOVERLAY_H

#include "AppTheme.h"
#include "StatsCollector.h"
#include <string>

void DrawSettingsOverlay(int sw, int sh, int fontSize, const AppTheme& theme,
                         bool tileEnabled[], StatsCollector& stats);

#endif
