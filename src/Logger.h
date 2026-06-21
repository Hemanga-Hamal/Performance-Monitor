#ifndef LOGGERV1_H
#define LOGGERV1_H

#include <windows.h>
#include <string>
#include <cstdio>

class LoggerV1 {
public:
    LoggerV1() noexcept;
    ~LoggerV1() noexcept;

    LoggerV1(const LoggerV1&) = delete;
    LoggerV1& operator=(const LoggerV1&) = delete;

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
