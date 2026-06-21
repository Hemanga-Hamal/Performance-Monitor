#include "ConfigManager.h"
#include <shlobj.h>
#include <cstdio>
#include <algorithm>

namespace {
    constexpr const wchar_t* CONFIG_FILENAME = L"\\PerfMon\\config.ini";
}

ConfigManager::ConfigManager() noexcept {
    resolvePath();
}

void ConfigManager::resolvePath() noexcept {
    wchar_t appData[MAX_PATH] = {};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) {
        mFilePath = appData;
        mFilePath += CONFIG_FILENAME;

        wchar_t dir[MAX_PATH] = {};
        wcscpy_s(dir, appData);
        wcscat_s(dir, L"\\PerfMon");
        CreateDirectoryW(dir, nullptr);
    }
}

bool ConfigManager::load() noexcept {
    if (mFilePath.empty()) return false;

    FILE* f = nullptr;
    _wfopen_s(&f, mFilePath.c_str(), L"r, ccs=UTF-8");
    if (!f) return false;

    wchar_t line[512];
    while (fgetws(line, sizeof(line) / sizeof(wchar_t), f)) {
        int index = 0, value = 0;
        if (swscanf_s(line, L"theme=%d", &index) == 1) {
            mConfig.themeIndex = (std::max)(0, (std::min)(2, index));
        }
        else if (swscanf_s(line, L"tile%d=%d", &index, &value) == 2) {
            if (index >= 0 && index < 5) mConfig.tileEnabled[index] = (value != 0);
        }
        else if (swscanf_s(line, L"windowX=%d", &value) == 1) {
            mConfig.windowX = value;
        }
        else if (swscanf_s(line, L"windowY=%d", &value) == 1) {
            mConfig.windowY = value;
        }
        else if (swscanf_s(line, L"windowW=%d", &value) == 1) {
            mConfig.windowW = value;
        }
        else if (swscanf_s(line, L"windowH=%d", &value) == 1) {
            mConfig.windowH = value;
        }
        else if (swscanf_s(line, L"disk%d=%d", &index, &value) == 2) {
            if (index >= 0 && index < 8) mConfig.diskEnabled[index] = (value != 0);
        }
        else if (swscanf_s(line, L"adapter%d=%d", &index, &value) == 2) {
            if (index >= 0 && index < 4) mConfig.adapterEnabled[index] = (value != 0);
        }
    }

    fclose(f);
    return true;
}

bool ConfigManager::save(const AppConfig& cfg) noexcept {
    if (mFilePath.empty()) return false;

    FILE* f = nullptr;
    _wfopen_s(&f, mFilePath.c_str(), L"w, ccs=UTF-8");
    if (!f) return false;

    fwprintf(f, L"theme=%d\n", cfg.themeIndex);
    for (int i = 0; i < 5; i++) {
        fwprintf(f, L"tile%d=%d\n", i, cfg.tileEnabled[i] ? 1 : 0);
    }
    fwprintf(f, L"windowX=%d\n", cfg.windowX);
    fwprintf(f, L"windowY=%d\n", cfg.windowY);
    fwprintf(f, L"windowW=%d\n", cfg.windowW);
    fwprintf(f, L"windowH=%d\n", cfg.windowH);
    for (int i = 0; i < 8; i++) {
        fwprintf(f, L"disk%d=%d\n", i, cfg.diskEnabled[i] ? 1 : 0);
    }
    for (int i = 0; i < 4; i++) {
        fwprintf(f, L"adapter%d=%d\n", i, cfg.adapterEnabled[i] ? 1 : 0);
    }

    fclose(f);
    return true;
}
