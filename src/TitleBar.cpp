#include "TitleBar.h"
#include <algorithm>

bool TitleBar::draw(int screenW, const AppTheme& theme, int fontSize) noexcept {
    Rectangle tb = {0, 0, static_cast<float>(screenW), static_cast<float>(TITLE_BAR_H)};
    DrawRectangleRec(tb, theme.tileBg);
    DrawLineEx({0, static_cast<float>(TITLE_BAR_H)}, {static_cast<float>(screenW), static_cast<float>(TITLE_BAR_H)}, 1.0f, theme.lineColor);

    int titleFontSize = std::max(fontSize - 2, 11);
    const char* titleText = "Performance Monitor";
    int tw = MeasureText(titleText, titleFontSize);
    DrawText(titleText, (screenW - tw) / 2, (TITLE_BAR_H - titleFontSize) / 2, titleFontSize, theme.titleText);

    int btnSize = TITLE_BAR_H - 8;
    Rectangle closeBtn = {static_cast<float>(screenW - btnSize - 6), 4.0f, static_cast<float>(btnSize), static_cast<float>(btnSize)};
    Vector2 mousePos = GetMousePosition();
    bool closeHover = CheckCollisionPointRec(mousePos, closeBtn);
    Color closeCol = closeHover ? theme.accentColor : theme.textMuted;
    DrawText("X", static_cast<int>(closeBtn.x + 6), static_cast<int>(closeBtn.y + 1), titleFontSize, closeCol);

    if (closeHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return true;
    }

    return false;
}
