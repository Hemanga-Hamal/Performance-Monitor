#ifndef LANDINGPAGE_H
#define LANDINGPAGE_H

#include "raylib.h"
#include "ThemeV1.h"
#include <algorithm>

enum AppState { LANDING, DASHBOARD };

inline void drawLandingPage(const ThemeV1& activeTheme, int sw, int sh, int titleSize, int fontSize,
                            int& selectedThemeIndex, ThemeV1& activeThemeRef, AppState& appState) {
    ClearBackground(activeTheme.landingBg);

    int heroY = sh / 5;
    const char* title = "Performance Monitor";
    int titleFont = titleSize * 2;
    int titleW = MeasureText(title, titleFont);
    DrawText(title, (sw - titleW) / 2, heroY, titleFont, activeTheme.titleText);

    const char* subtitle = "Select a theme to get started";
    int subFont = static_cast<int>(fontSize * 0.95f);
    int subW = MeasureText(subtitle, subFont);
    DrawText(subtitle, (sw - subW) / 2, heroY + titleFont + 20, subFont, activeTheme.textSecondary);

    const char* themeNames[] = {"Dark", "Light", "High Contrast"};
    ThemeV1 themePreviews[] = {ThemeV1::Dark(), ThemeV1::Light(), ThemeV1::HighContrast()};

    float cardScale = (std::min)(sw / 1300.0f, sh / 850.0f);
    cardScale = (std::max)((std::min)(cardScale, 2.0f), 0.5f);
    int cardW = static_cast<int>(200 * cardScale);
    int cardH = static_cast<int>(140 * cardScale);
    int spacing = static_cast<int>(24 * cardScale);
    int totalCardsW = cardW * 3 + spacing * 2;
    int startX = (sw - totalCardsW) / 2;
    int cardY = heroY + titleFont + 80;

    for (int i = 0; i < 3; i++) {
        Rectangle card = {static_cast<float>(startX + i * (cardW + spacing)),
                          static_cast<float>(cardY), static_cast<float>(cardW), static_cast<float>(cardH)};
        bool isSelected = (selectedThemeIndex == i);
        bool isHovered = CheckCollisionPointRec(GetMousePosition(), card);

        Rectangle shadow = {card.x + 4, card.y + 4, card.width, card.height};
        DrawRectangleRounded(shadow, 0.12f, 12, activeTheme.tileShadow);

        DrawRectangleRounded(card, 0.12f, 12, themePreviews[i].tileBg);

        Color borderCol = isSelected ? activeTheme.accentColor :
                          (isHovered ? activeTheme.accentHover : themePreviews[i].tileBorder);
        float borderW = isSelected ? 2.5f : 1.0f;
        DrawRectangleRoundedLinesEx(card, 0.12f, 12, borderW, borderCol);

        if (isSelected) {
            Rectangle glow = {card.x - 1, card.y - 1, card.width + 2, card.height + 2};
            DrawRectangleRoundedLinesEx(glow, 0.12f, 12, 3.0f,
                {activeTheme.accentColor.r, activeTheme.accentColor.g, activeTheme.accentColor.b, 40});
        }

        int tf = static_cast<int>(fontSize * cardScale * 0.9f);
        if (tf < 11) tf = 11;
        DrawText(themeNames[i], static_cast<int>(card.x + 18 * cardScale),
                 static_cast<int>(card.y + 16 * cardScale), tf, themePreviews[i].titleText);

        float swatchW = 44 * cardScale;
        float swatchH = 32 * cardScale;
        float swatchGap = 12 * cardScale;
        float sy = card.y + 50 * cardScale;
        float sx = card.x + 18 * cardScale;
        Rectangle s1 = {sx, sy, swatchW, swatchH};
        Rectangle s2 = {sx + swatchW + swatchGap, sy, swatchW, swatchH};
        Rectangle s3 = {sx + (swatchW + swatchGap) * 2, sy, swatchW, swatchH};
        DrawRectangleRounded(s1, 0.2f, 8, themePreviews[i].barForeground);
        DrawRectangleRounded(s2, 0.2f, 8, themePreviews[i].gaugeArcActive);
        DrawRectangleRounded(s3, 0.2f, 8, themePreviews[i].barBackground);

        if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            selectedThemeIndex = i;
            activeThemeRef = themePreviews[i];
        }
    }

    int btnW = static_cast<int>(220 * cardScale);
    int btnH = static_cast<int>(52 * cardScale);
    if (btnH < 34) btnH = 34;
    Rectangle startBtn = {static_cast<float>((sw - btnW) / 2),
                          static_cast<float>(sh - sh / 4),
                          static_cast<float>(btnW), static_cast<float>(btnH)};
    bool btnHover = CheckCollisionPointRec(GetMousePosition(), startBtn);
    Color btnColor = btnHover ? activeTheme.accentHover : activeTheme.accentColor;

    Rectangle btnShadow = {startBtn.x + 2, startBtn.y + 3, startBtn.width, startBtn.height};
    DrawRectangleRounded(btnShadow, 0.25f, 12, {0, 0, 0, 50});
    DrawRectangleRounded(startBtn, 0.25f, 12, btnColor);

    const char* btnText = "Start Monitoring";
    int btnFont = static_cast<int>(fontSize * cardScale * 0.95f);
    if (btnFont < 12) btnFont = 12;
    int btnTextW = MeasureText(btnText, btnFont);
    DrawText(btnText, static_cast<int>(startBtn.x + (btnW - btnTextW) / 2),
             static_cast<int>(startBtn.y + (btnH - btnFont) / 2), btnFont,
             activeTheme.windowBg);

    if (btnHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        appState = DASHBOARD;
    }
}

#endif
