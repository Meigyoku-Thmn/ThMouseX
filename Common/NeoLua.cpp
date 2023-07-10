#include "framework.h"
#include <string>
#include "macro.h"
#include <iostream>
#include <wrl/client.h>
#include <metahost.h>
#include <comdef.h>

#include "DataTypes.h"
#include "NeoLua.h"
#include "Log.h"
#include "Variables.h"

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
void NeoLua_SetOnClose(DWORD address) {
    *(PDWORD)&onClose = address;
}

DWORD positionAddress;
void NeoLua_SetPositionAddress(DWORD address) {
    positionAddress = address;
}

PointDataType NeoLua_GetDataType() {
    return g_currentConfig.PosDataType;
}

void NeoLua_OpenConsole() {
    note::OpenConsole();
}

namespace common::neolua {
    DWORD GetPositionAddress() {
        return positionAddress;
    }

    HMODULE mscoree;
    void Initialize() {
        if (g_currentConfig.ScriptingMethodToFindAddress != ScriptingMethod::NeoLua)
            return;

        mscoree = LoadLibraryW(L"mscoree.dll");
        if (!mscoree) {
            log::ToFile(TAG " Failed to load mscoree.dll.");
            return;
        }

        auto _CLRCreateInstance = (decltype(&CLRCreateInstance))GetProcAddress(mscoree, "CLRCreateInstance");
        if (!_CLRCreateInstance) {
            log::ToFile(TAG " Failed to import mscoree.dll|CLRCreateInstance.");
            return;
        }

        ComPtr<ICLRMetaHost> metaHost;
        auto result = _CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&metaHost));
        IfFailedReturnWithLog(TAG, "Cannot create ICLRMetaHost instance", result);

        ComPtr<IEnumUnknown> enumerator;
        for (auto i = 0; i < 5; i++) {
            // It seems that EnumerateLoadedRuntimes call CreateToolhelp32Snapshot internally, which can fail with ERROR_BAD_LENGTH for no obvious reason. So, just retry if that's the case.
            result = metaHost->EnumerateLoadedRuntimes(GetCurrentProcess(), &enumerator);
            if (WIN32_FROM_HRESULT(result) != ERROR_BAD_LENGTH)
                break;
        }
        IfFailedReturnWithLog(TAG, "Cannot enumerate loaded clr runtimes", result);
        ComPtr<ICLRRuntimeInfo> runtimeInfo;
        ULONG count = 0;
        result = enumerator->Next(1, (IUnknown**)&runtimeInfo, &count);
        IfFailedReturnWithLog(TAG, "Cannot enumerate on IEnum", result);
        if (count == 0)
            ReturnWithErrorMessage("There is no loaded clr runtime.");

        ComPtr<ICLRRuntimeHost> runtimeHost;
        result = runtimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_PPV_ARGS(&runtimeHost));
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

    void Uninitialize() {
        if (onClose != nullptr)
            onClose();
        onClose = nullptr;
        positionAddress = NULL;
        SAFE_FREE_LIB(mscoree);
    }
}
