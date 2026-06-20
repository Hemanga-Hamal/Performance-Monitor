#ifndef CONFIGV1_H
#define CONFIGV1_H

#include <windows.h>
#include <string>

struct AppConfig {
    int themeIndex{0};
    bool tileEnabled[5]{true, true, true, true, true};
    int windowX{-1};
    int windowY{-1};
    int windowW{1200};
    int windowH{800};
};

class ConfigV1 {
public:
    ConfigV1() noexcept;

    ConfigV1(const ConfigV1&) = delete;
    ConfigV1& operator=(const ConfigV1&) = delete;

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
