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

using namespace std;
using namespace Microsoft::WRL;

#define ReturnWithErrorMessageAndDetail(message, detail) { \
    FileLog("[NeoLua] %s: %s", message, detail); \
    return; \
}

#define ReturnWithErrorMessage(message) { \
    FileLog("[NeoLua] %s", message); \
    return; \
}

#define IfFailedReturnWithLog(result, message) { \
    if (FAILED(result)) \
        ReturnWithErrorMessageAndDetail(message, _com_error(result).ErrorMessage()); \
}

// do not use CLRCreateInstance directly
auto _CLRCreateInstance = (decltype(&CLRCreateInstance))GetProcAddress(LoadLibraryW(L"mscoree.dll"), "CLRCreateInstance");

void OnClose();
decltype(&OnClose) onClose;
DLLEXPORT_C void SetOnClose(DWORD address) {
    *(PDWORD)&onClose = address;
}

DWORD positionAddress;
DLLEXPORT_C void SetPositionAddress(DWORD address) {
    positionAddress = address;
}
export DWORD NeoLua_GetPositionAddress() {
    return positionAddress;
}

DLLEXPORT_C PointDataType GetDataType() {
    return g_currentConfig.PosDataType;
}

export DLLEXPORT void InitializeNeoLua() {
    if (g_currentConfig.ScriptingMethodToFindAddress != ScriptingMethod::NeoLua)
        return;
    if (_CLRCreateInstance == nullptr) {
        FileLog("[NeoLua] %s", "Failed to import mscoree.dll|CLRCreateInstance.");
        return;
    }

    ComPtr<ICLRMetaHost> metaHost;
    auto result = _CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&metaHost));
    IfFailedReturnWithLog(result, "Cannot create ICLRMetaHost instance");

    ComPtr<IEnumUnknown> enumerator;
    result = metaHost->EnumerateLoadedRuntimes(GetCurrentProcess(), &enumerator);
    IfFailedReturnWithLog(result, "Cannot enumerate loaded clr runtimes");
    ComPtr<ICLRRuntimeInfo> runtimeInfo;
    ULONG count = 0;
    enumerator->Next(1, (IUnknown**)&runtimeInfo, &count);
    if (count == 0)
        ReturnWithErrorMessage("There is no loaded clr runtime");

    ComPtr<ICLRRuntimeHost> runtimeHost;
    result = runtimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_PPV_ARGS(&runtimeHost));
    IfFailedReturnWithLog(result, "Cannot get ICLRRuntimeHost instance");

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
    IfFailedReturnWithLog(result, "Failed to invoke NeoLuaBootstrap.Handlers.OnInit");
}

export DLLEXPORT void UninitializeNeoLua() {
    if (onClose != nullptr)
        onClose();
    onClose = nullptr;
    positionAddress = NULL;
}