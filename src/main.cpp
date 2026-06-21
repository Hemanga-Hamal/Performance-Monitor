#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI
#define NOUSER

#include <windows.h>
#include "WindowManager.h"

int main() {
    return WindowManager::run();
}
