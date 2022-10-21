module;

#include "framework.h"
#include <string>
#include "macro.h"
#include <iostream>
#include <wrl/client.h>
#include <metahost.h>
#include <comdef.h>

export module common.neolua;

import common.datatype;
import common.var;
import common.log;

namespace note = common::log;

using namespace std;
using namespace Microsoft::WRL;

#define TAG "[NeoLua] "

#define ReturnWithErrorMessage(message) { \
    note::ToFile(TAG message); \
    return; \
}0

#define IfFailedReturnWithLog(tag, message, result) if (FAILED(result)) { \
    note::HResultToFile(tag message, result); \
    return; \
}0

void OnClose();
decltype(&OnClose) onClose;
DLLEXPORT_C void Common_NeoLua_SetOnClose(DWORD address) {
    *(PDWORD)&onClose = address;
}

DWORD positionAddress;
DLLEXPORT_C void Common_NeoLua_SetPositionAddress(DWORD address) {
    positionAddress = address;
}

DLLEXPORT_C PointDataType Common_NeoLua_GetDataType() {
    return g_currentConfig.PosDataType;
}

DLLEXPORT_C void Common_NeoLua_OpenConsole() {
    note::OpenConsole();
}

// do not use CLRCreateInstance directly
auto _CLRCreateInstance = (decltype(&CLRCreateInstance))GetProcAddress(LoadLibraryW(L"mscoree.dll"), "CLRCreateInstance");

namespace common::neolua {
    export DWORD GetPositionAddress() {
        return positionAddress;
    }

    export DLLEXPORT void Initialize() {
        if (g_currentConfig.ScriptingMethodToFindAddress != ScriptingMethod::NeoLua)
            return;
        if (_CLRCreateInstance == nullptr) {
            log::ToFile(TAG " %s", "Failed to import mscoree.dll|CLRCreateInstance.");
            return;
        }

        ComPtr<ICLRMetaHost> metaHost;
        auto result = _CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(metaHost.GetAddressOf()));
        IfFailedReturnWithLog(TAG, "Cannot create ICLRMetaHost instance", result);

        ComPtr<IEnumUnknown> enumerator;
        for (auto i = 0; i < 5; i++) {
            // It seems that EnumerateLoadedRuntimes call CreateToolhelp32Snapshot internally, which can fail with ERROR_BAD_LENGTH for no obvious reason. So, just retry if that's the case.
            result = metaHost->EnumerateLoadedRuntimes(GetCurrentProcess(), enumerator.GetAddressOf());
            if (WIN32_FROM_HRESULT(result) != ERROR_BAD_LENGTH)
                break;
        }
        IfFailedReturnWithLog(TAG, "Cannot enumerate loaded clr runtimes", result);
        ComPtr<ICLRRuntimeInfo> runtimeInfo;
        ULONG count = 0;
        result = enumerator->Next(1, (IUnknown**)runtimeInfo.GetAddressOf(), &count);
        IfFailedReturnWithLog(TAG, "Cannot enumerate on IEnum", result);
        if (count == 0)
            ReturnWithErrorMessage("There is no loaded clr runtime.");

        ComPtr<ICLRRuntimeHost> runtimeHost;
        result = runtimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_PPV_ARGS(runtimeHost.GetAddressOf()));
        IfFailedReturnWithLog(TAG, "Cannot get ICLRRuntimeHost instance", result);

        auto bootstrapDllPath = wstring(g_currentModuleDirPath) + L"/NeoLuaBootstrap.dll";
        auto scriptPath = wstring(g_currentModuleDirPath) + L"/ConfigScripts/" + g_currentConfig.ProcessName + L".lua";
        DWORD _;
        result = runtimeHost->ExecuteInDefaultAppDomain(
            bootstrapDllPath.c_str(),
            L"NeoLuaBootstrap.Handlers",
            L"OnInit",
            scriptPath.c_str(),
            (PDWORD)&_
        );
        IfFailedReturnWithLog(TAG, "Failed to invoke NeoLuaBootstrap.Handlers.OnInit", result);
    }

    export DLLEXPORT void Uninitialize() {
        if (onClose != nullptr)
            onClose();
        onClose = nullptr;
        positionAddress = NULL;
    }
}
