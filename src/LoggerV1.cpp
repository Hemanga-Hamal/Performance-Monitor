#include "LoggerV1.h"
#include <ctime>

LoggerV1::LoggerV1() noexcept {}

LoggerV1::~LoggerV1() noexcept {
    stop();
}

bool LoggerV1::start(const wchar_t* basePath) noexcept {
    if (mFile) stop();

    mPath = basePath;

    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t filename[MAX_PATH];
    swprintf_s(filename, MAX_PATH, L"%s\\perfmon_%04d%02d%02d_%02d%02d%02d.csv",
               basePath, st.wYear, st.wMonth, st.wDay,
               st.wHour, st.wMinute, st.wSecond);

    _wfopen_s(&mFile, filename, L"w, ccs=UTF-8");
    if (!mFile) return false;

    fwprintf(mFile, L"Timestamp,CPU_Freq_MHz,CPU_Util_Pct,RAM_Used_GB,RAM_Util_Pct,GPU_Util_Pct,Wifi_Send_Mbps,Wifi_Recv_Mbps,Eth_Send_Mbps,Eth_Recv_Mbps\n");
    fflush(mFile);
    return true;
}

void LoggerV1::stop() noexcept {
    if (mFile) {
        fclose(mFile);
        mFile = nullptr;
    }
}

void LoggerV1::writeRow(float cpuFreq, float cpuUtil, float ramUsed, float ramUtil,
                         float gpuUtil, float wifiSend, float wifiRecv,
                         float ethSend, float ethRecv) noexcept {
    if (!mFile) return;

    SYSTEMTIME st;
    GetLocalTime(&st);
    fwprintf(mFile, L"%04d-%02d-%02d %02d:%02d:%02d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
             cpuFreq, cpuUtil, ramUsed, ramUtil, gpuUtil,
             wifiSend, wifiRecv, ethSend, ethRecv);
    fflush(mFile);
}
