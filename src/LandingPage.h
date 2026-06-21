#ifndef LANDINGPAGE_H
#define LANDINGPAGE_H

#include "AppTheme.h"

enum AppState { LANDING, DASHBOARD };

bool DrawLandingPage(int sw, int sh, int titleSize, int fontSize,
                     const AppTheme& theme, int& selectedThemeIndex,
                     AppTheme& activeTheme, AppState& appState);

#endif
