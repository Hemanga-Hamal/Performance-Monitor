#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <dwmapi.h>

static constexpr int TITLE_BAR_H = 32;

namespace {
    WNDPROC gOriginalWndProc = nullptr;
    bool gInTitleBar(int px, int py, int width, int barH) {
        return py >= 0 && py < barH && px < width - barH + 8 + 6;
    }
}

LRESULT CALLBACK TitleBarSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCCALCSIZE: {
        if (wParam == TRUE) {
            NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
            LRESULT res = CallWindowProcW(gOriginalWndProc, hwnd, msg, wParam, lParam);
            params->rgrc[0].top -= 1;
            return 0;
        }
        break;
    }
    case WM_NCHITTEST: {
        POINT pt = {static_cast<SHORT>(lParam & 0xFFFF), static_cast<SHORT>(lParam >> 16)};
        ScreenToClient(hwnd, &pt);
        RECT r; GetClientRect(hwnd, &r);
        if (gInTitleBar(pt.x, pt.y, r.right, TITLE_BAR_H)) {
            return HTCAPTION;
        }
        break;
    }
    }
    return CallWindowProcW(gOriginalWndProc, hwnd, msg, wParam, lParam);
}

extern "C" void InitTitleBarNative() {
    HWND hwnd = FindWindowW(nullptr, L"Performance Monitor");
    if (!hwnd) return;

    LONG style = GetWindowLongW(hwnd, GWL_STYLE);
    style |= WS_CAPTION | WS_THICKFRAME;
    SetWindowLongW(hwnd, GWL_STYLE, style);

    MARGINS margins = {0, 0, 0, 1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    gOriginalWndProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(TitleBarSubclassProc)));
}
