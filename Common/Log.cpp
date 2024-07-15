#include "framework.h"
#include "macro.h"
#include <iostream>
#include <chrono>
#include <cstdio>
#include <string>
#include <comdef.h>
#include <wrl/client.h>
#include <DirectX9/Include/DxErr.h>

#include "Log.h"
#include "Helper.Encoding.h"
#include "ErrorMsg.h"
#include "Variables.h"

namespace encoding = common::helper::encoding;
namespace errormsg = common::errormsg;

using namespace std;
using namespace Microsoft::WRL;

static tm& GetTimeNow() {
    auto now = time(nullptr);
    return *localtime(&now);
}

namespace common::log {
    void OpenConsole() {
        if (AllocConsole() == FALSE)
            return;
#pragma warning(push)
#pragma warning(disable: 6031)
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
#pragma warning(pop)
        printf("Debugging Window:\n\n");
    }

    FILE* logFile;
    wstring logPath;
    string processName;
    void ToFile(const char* _Format, ...) {
        va_list args;
        va_start(args, _Format);
        if (!logFile) {
            if (logPath.size() == 0) {
                logPath = wstring(g_currentModuleDirPath) + L"/log.txt";
                processName = encoding::ConvertToUtf8(g_currentConfig.ProcessName);
            }
            logFile = _wfsopen(logPath.c_str(), L"a+", _SH_DENYNO);
            if (logFile != nullptr)
                setvbuf(logFile, nullptr, _IONBF, 0);
        }
        if (logFile != nullptr) {
            auto const& now = GetTimeNow();
            fprintf(logFile, "[%s %02d/%02d/%02d %02d:%02d:%02d] ",
                processName.c_str(),
                now.tm_mday, now.tm_mon + 1, now.tm_year + 1900,
                now.tm_hour, now.tm_min, now.tm_sec);
            vfprintf(logFile, _Format, args);
            fprintf(logFile, "\n");
        }
        va_end(args);
    }

    void DxErrToFile(const char* message, HRESULT hResult) {
        auto errorStr = DXGetErrorStringA(hResult);
        if (errorStr == nullptr) {
            HResultToFile(message, hResult);
            return;
        }
        auto errorDes = DXGetErrorDescriptionA(hResult);
        auto description = errorDes != nullptr ? string(errorStr) + ": " + errorDes : string(errorStr);
#if _DEBUG
        ToConsole("%s: %s", message, description.c_str());
#endif
        ToFile("%s: %s", message, description.c_str());
    }

    void HResultToFile(const char* message, HRESULT hResult) {
        ComPtr<IErrorInfo> errorInfo;
        std::ignore = GetErrorInfo(0, &errorInfo);
        _com_error error(hResult, errorInfo.Get(), true);
        auto description = error.Description();
        if (description.length() > 0) {
#if _DEBUG
            ToConsole("%s: %s", message, description);
#endif
            ToFile("%s: %s", message, description);
            return;
        }
        auto errorMessage = string(error.ErrorMessage());
#if _DEBUG
        ToConsole("%s: %s", message, errorMessage.c_str());
#endif
        ToFile("%s: %s", message, errorMessage.c_str());
        if (errorMessage.starts_with("IDispatch error") || errorMessage.starts_with("Unknown error")) {
            errorMessage = errormsg::GuessErrorsFromHResult(hResult);
            if (errorMessage != "") {
#if _DEBUG
                ToConsole("%s", errorMessage.c_str());
#endif
                ToFile(errorMessage.c_str());
            }
        }
    }

    void LastErrorToFile(const char* message) {
        _com_error error(GetLastError());
        auto detail = error.ErrorMessage();
#if _DEBUG
        ToConsole("%s: %s", message, detail);
#endif
        ToFile("%s: %s", message, detail);
    }

    void ToConsole(const char* _Format, ...) {
        OpenConsole();
        va_list args;
        va_start(args, _Format);
        vprintf(_Format, args);
        printf("\n");
        va_end(args);
    }

    void FpsToConsole() {
        using namespace chrono;
        static time_point<steady_clock> oldTime = high_resolution_clock::now();
        static int fps; fps++;

        if (duration_cast<seconds>(high_resolution_clock::now() - oldTime) >= seconds{1}) {
            oldTime = high_resolution_clock::now();
            ToConsole("FPS: %d", fps);
            fps = 0;
        }
    }
}
