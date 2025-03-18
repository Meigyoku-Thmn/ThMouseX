#include <Windows.h>
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

namespace errormsg = common::errormsg;

using namespace std;
using namespace Microsoft::WRL;

namespace common::log {
    tm GetTimeNow() {
        tm rs;
        auto now = time(nil);
        localtime_s(&rs, &now);
        return rs;
    }

    void OpenConsole() {
        if (AllocConsole() == FALSE)
            return;
#pragma warning(push)
#pragma warning(disable: 6031)
        ignore = freopen("conin$", "r", stdin);
        ignore = freopen("conout$", "w", stdout);
        ignore = freopen("conout$", "w", stderr);
#pragma warning(pop)
        printf("Debugging Window:\n\n");
    }

    void DxErrToFile(const char* message, HRESULT hResult) {
        auto errorStr = DXGetErrorStringA(hResult);
        if (errorStr == nil) {
            HResultToFile(message, hResult);
            return;
        }
        auto errorDes = DXGetErrorDescriptionA(hResult);
        auto description = errorDes != nil ? string(errorStr) + ": " + errorDes : string(errorStr);
        ToFile("%s: %s", message, description.c_str());
    }

    void HResultToFile(const char* message, HRESULT hResult) {
        ComPtr<IErrorInfo> errorInfo;
        ignore = GetErrorInfo(0, &errorInfo);
        _com_error error(hResult, errorInfo.Get(), true);
        ComErrToFile(message, error);
    }

    void ComErrToFile(const char* message, const _com_error& error) {
        auto description = error.Description();
        if (description.length() > 0) {
            ToFile("%s: %s", message, scast<char*>(description));
            return;
        }
        auto errorMessage = string(error.ErrorMessage());
        ToFile("%s: %s", message, errorMessage.c_str());
        if (errorMessage.starts_with("IDispatch error") || errorMessage.starts_with("Unknown error")) {
            errorMessage = errormsg::GuessErrorsFromHResult(error.Error());
            if (errorMessage != "") {
                ToFile(errorMessage.c_str());
            }
        }
    }

    void LastErrorToFile(const char* message) {
        _com_error error(GetLastError());
        auto detail = error.ErrorMessage();
        ToFile("%s: %s", message, detail);
    }

    void FpsToConsole() {
        using namespace chrono;
        static time_point<steady_clock> oldTime = high_resolution_clock::now();
        static int fps; fps++;

        if (duration_cast<seconds>(high_resolution_clock::now() - oldTime) >= seconds{ 1 }) {
            oldTime = high_resolution_clock::now();
            ToConsole("FPS: %d", fps);
            fps = 0;
        }
    }
}
