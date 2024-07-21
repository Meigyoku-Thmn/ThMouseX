#pragma once
#include "framework.h"
#include "macro.h"
#include "Variables.h"
#include "Helper.Encoding.h"

#include <string>
#include <cstdio>

namespace common::log {
    extern FILE* __logFile;
    extern std::wstring __logPath;
    extern std::string __processName;
    tm GetTimeNow();
    void OpenConsole();
    template <typename... Targs>
    void ToFile(const char* _Format, Targs... args) {
        namespace encoding = common::helper::encoding;
        if (!__logFile) {
            if (__logPath.size() == 0) {
                __logPath = std::wstring(g_currentModuleDirPath) + L"/log.txt";
                __processName = encoding::ConvertToUtf8(g_currentConfig.ProcessName);
            }
            __logFile = _wfsopen(__logPath.c_str(), L"a+", _SH_DENYNO);
            if (__logFile != nil)
                setvbuf(__logFile, nil, _IONBF, 0);
        }
        if (__logFile != nil) {
            auto const& now = GetTimeNow();
            fprintf(__logFile, "[%s %02d/%02d/%02d %02d:%02d:%02d] ",
                __processName.c_str(),
                now.tm_mday, now.tm_mon + 1, now.tm_year + 1900,
                now.tm_hour, now.tm_min, now.tm_sec);
            fprintf(__logFile, _Format, &args...);
            fprintf(__logFile, "\n");
        }
    }
    void DxErrToFile(const char* message, HRESULT hResult);
    void HResultToFile(const char* message, HRESULT hResult);
    void LastErrorToFile(const char* message);
    template <typename... Targs>
    void ToConsole(const char* _Format, Targs... args) {
        OpenConsole();
        printf(_Format, &args...);
        printf("\n");
    }
    void FpsToConsole();
}
