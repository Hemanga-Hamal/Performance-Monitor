#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <windows.h>
#include <string>

struct AppConfig {
    int themeIndex{0};
    bool tileEnabled[5]{true, true, true, true, true};
    int windowX{-1};
    int windowY{-1};
    int windowW{1200};
    int windowH{800};
    bool diskEnabled[8]{true, true, true, true, true, true, true, true};
    bool adapterEnabled[4]{true, true, true, true};
};

class ConfigManager {
public:
    ConfigManager() noexcept;

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    [[nodiscard]] bool load() noexcept;
    [[nodiscard]] bool save(const AppConfig& cfg) noexcept;
    [[nodiscard]] const AppConfig& get() const noexcept { return mConfig; }
    void set(const AppConfig& cfg) noexcept { mConfig = cfg; }

private:
    AppConfig mConfig;
    std::wstring mFilePath;

    void resolvePath() noexcept;
};

#endif
