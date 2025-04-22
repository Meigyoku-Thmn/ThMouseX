#pragma once
#include <Windows.h>
#include "macro.h"
#include "Variables.h"
#include "Helper.Encoding.h"

#include <string>
#include <cstdio>
#include <utility>
#include <comdef.h>

namespace common::log {
    class __LogData {
    public:
        static __LogData& Get() {
            static __LogData data{};
            return data;
        }
        std::wstring path = std::wstring(g_currentModuleDirPath) + L"/log.txt";
        std::string processName = common::helper::encoding::ConvertToUtf8(g_currentProcessName);
        FILE* file = _wfsopen(path.c_str(), L"a+", _SH_DENYNO);
    private:
        __LogData() {
            if (file != nil) setvbuf(file, nil, _IOFBF, 4096);
        }
        ~__LogData() {
            if (file != nil) fclose(file);
        }
    };
    tm GetTimeNow();
    void OpenConsole();
    template <typename... Targs>
    void ToFile(PCSTR _Format, Targs&&... args) {
        auto& logData = __LogData::Get();
        if (logData.file == nil)
            return;
        auto const& now = GetTimeNow();
        fprintf(logData.file, "[%s %02d/%02d/%02d %02d:%02d:%02d] ",
            logData.processName.c_str(),
            now.tm_mday, now.tm_mon + 1, now.tm_year + 1900,
            now.tm_hour, now.tm_min, now.tm_sec);
        fprintf(logData.file, _Format, std::forward<Targs>(args)...);
        fprintf(logData.file, "\n");
        fflush(logData.file);
    }
    void DxErrToFile(PCSTR message, HRESULT hResult);
    void HResultToFile(PCSTR message, HRESULT hResult);
    void ComErrToFile(PCSTR message, const _com_error& error);
    void LastErrorToFile(PCSTR message);
    template <typename... Targs>
    void ToConsole(PCSTR _Format, Targs&&... args) {
        OpenConsole();
        printf(_Format, std::forward<Targs>(args)...);
        printf("\n");
    }
    void FpsToConsole();
}
