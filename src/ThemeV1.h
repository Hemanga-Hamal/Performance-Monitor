#ifndef THEMEV1_H
#define THEMEV1_H

#include "raylib.h"

struct ThemeV1 {
    Color windowBg;
    Color tileBg;
    Color tileBorder;
    Color tileShadow;
    Color titleText;
    Color textPrimary;
    Color textSecondary;
    Color textMuted;
    Color lineColor;
    Color barBackground;
    Color barForeground;
    Color barForegroundDim;
    Color gaugeArcBg;
    Color gaugeArcActive;
    Color gaugeArcGlow;
    Color gaugeText;
    Color landingBg;
    Color accentColor;
    Color accentHover;
    Color panelOverlay;
    Color toggleActive;
    Color toggleInactive;
    Color scrollbarColor;

    static ThemeV1 Dark() {
        ThemeV1 t;
        t.windowBg       = { 16,  16,  20, 255};
        t.tileBg         = { 24,  24,  32, 255};
        t.tileBorder     = { 44,  44,  56, 255};
        t.tileShadow     = {  0,   0,   0,  60};
        t.titleText      = { 230, 230, 240, 255};
        t.textPrimary    = { 210, 210, 225, 255};
        t.textSecondary  = { 140, 140, 155, 255};
        t.textMuted      = {  90,  90, 100, 255};
        t.lineColor      = { 52,  52,  64, 255};
        t.barBackground  = { 40,  40,  50, 255};
        t.barForeground  = {  90, 180, 255, 255};
        t.barForegroundDim = { 50, 120, 180, 255};
        t.gaugeArcBg     = { 36,  36,  46, 255};
        t.gaugeArcActive = {  90, 180, 255, 255};
        t.gaugeArcGlow   = {  90, 180, 255,  40};
        t.gaugeText      = { 230, 230, 240, 255};
        t.landingBg      = { 12,  12,  16, 255};
        t.accentColor    = {  90, 180, 255, 255};
        t.accentHover    = { 120, 200, 255, 255};
        t.panelOverlay   = { 16,  16,  22, 248};
        t.toggleActive   = {  90, 180, 255, 255};
        t.toggleInactive = {  50,  50,  62, 255};
        t.scrollbarColor = { 100, 100, 120, 180};
        return t;
    }

    static ThemeV1 Light() {
        ThemeV1 t;
        t.windowBg       = { 238, 238, 244, 255};
        t.tileBg         = { 250, 250, 254, 255};
        t.tileBorder     = { 210, 210, 220, 255};
        t.tileShadow     = {   0,   0,   0,  30};
        t.titleText      = {  24,  24,  34, 255};
        t.textPrimary    = {  38,  38,  48, 255};
        t.textSecondary  = { 120, 120, 130, 255};
        t.textMuted      = { 170, 170, 180, 255};
        t.lineColor      = { 225, 225, 232, 255};
        t.barBackground  = { 225, 225, 232, 255};
        t.barForeground  = {  55, 130, 220, 255};
        t.barForegroundDim = { 120, 170, 230, 255};
        t.gaugeArcBg     = { 218, 218, 226, 255};
        t.gaugeArcActive = {  55, 130, 220, 255};
        t.gaugeArcGlow   = {  55, 130, 220,  35};
        t.gaugeText      = {  24,  24,  34, 255};
        t.landingBg      = { 228, 228, 236, 255};
        t.accentColor    = {  55, 130, 220, 255};
        t.accentHover    = {  80, 155, 235, 255};
        t.panelOverlay   = { 240, 240, 246, 248};
        t.toggleActive   = {  55, 130, 220, 255};
        t.toggleInactive = { 190, 190, 200, 255};
        t.scrollbarColor = { 160, 160, 175, 180};
        return t;
    }

    static ThemeV1 HighContrast() {
        ThemeV1 t;
        t.windowBg       = {   0,   0,   0, 255};
        t.tileBg         = {   8,   8,   8, 255};
        t.tileBorder     = { 255, 255, 255, 255};
        t.tileShadow     = { 255, 255, 255,  20};
        t.titleText      = { 255, 255,  80, 255};
        t.textPrimary    = { 255, 255, 255, 255};
        t.textSecondary  = { 200, 200, 200, 255};
        t.textMuted      = { 140, 140, 140, 255};
        t.lineColor      = { 255, 255, 255, 255};
        t.barBackground  = {  25,  25,  25, 255};
        t.barForeground  = {   0, 255, 255, 255};
        t.barForegroundDim = {  0, 180, 180, 255};
        t.gaugeArcBg     = {  18,  18,  18, 255};
        t.gaugeArcActive = {   0, 255,  80, 255};
        t.gaugeArcGlow   = {   0, 255,  80,  40};
        t.gaugeText      = { 255, 255, 255, 255};
        t.landingBg      = {   0,   0,   0, 255};
        t.accentColor    = {   0, 255, 255, 255};
        t.accentHover    = {  80, 255, 255, 255};
        t.panelOverlay   = {   5,   5,   5, 250};
        t.toggleActive   = {   0, 255, 255, 255};
        t.toggleInactive = {  40,  40,  40, 255};
        t.scrollbarColor = { 200, 200, 200, 200};
        return t;
    }
};

#endif
