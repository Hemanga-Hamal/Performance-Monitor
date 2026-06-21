#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>
#include <string>
#include <cstdio>

class Logger {
public:
    Logger() noexcept;
    ~Logger() noexcept;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    [[nodiscard]] bool start(const wchar_t* basePath) noexcept;
    void stop() noexcept;
    [[nodiscard]] bool isLogging() const noexcept { return mFile != nullptr; }

    void writeRow(float cpuFreq, float cpuUtil, float ramUsed, float ramUtil,
                  float gpuUtil, float wifiSend, float wifiRecv,
                  float ethSend, float ethRecv) noexcept;

private:
    FILE* mFile{nullptr};
    std::wstring mPath;
};

#endif
