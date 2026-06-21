#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "raylib.h"
#include "AppTheme.h"

constexpr int TITLE_BAR_H = 32;

class TitleBar {
public:
    [[nodiscard]] bool draw(int screenW, const AppTheme& theme, int fontSize) noexcept;
};

#endif
