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
#include "LuaApi.h"

namespace note = common::log;

using namespace std;
using namespace Microsoft::WRL;

#define TAG "[NeoLua] "

namespace common::neolua {
    DWORD GetPositionAddress() {
        return Lua_GetPositionAddress();
    }

    void Initialize() {
        if (g_currentConfig.ScriptType != ScriptType::NeoLua)
            return;

        auto mscoree = GetModuleHandleW(L"mscoree.dll");
        if (!mscoree) {
            log::LastErrorToFile(TAG " Failed to load mscoree.dll.");
            return;
        }

        auto _CLRCreateInstance = (decltype(&CLRCreateInstance))GetProcAddress(mscoree, "CLRCreateInstance");
        if (!_CLRCreateInstance) {
            log::LastErrorToFile(TAG " Failed to import mscoree.dll|CLRCreateInstance.");
            return;
        }

        ComPtr<ICLRMetaHost> metaHost;
        auto result = _CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&metaHost));
        if (FAILED(result)) {
            note::HResultToFile(TAG "Cannot create ICLRMetaHost instance", result); return;
        }

        ComPtr<IEnumUnknown> enumerator;
        for (auto i = 0; i < 5; i++) {
            // It seems that EnumerateLoadedRuntimes call CreateToolhelp32Snapshot internally, which can fail with ERROR_BAD_LENGTH for no obvious reason. So, just retry if that's the case.
            result = metaHost->EnumerateLoadedRuntimes(GetCurrentProcess(), &enumerator);
            if (WIN32_FROM_HRESULT(result) != ERROR_BAD_LENGTH)
                break;
        }
        if (FAILED(result)) {
            note::HResultToFile(TAG "Cannot enumerate loaded clr runtimes", result); return;
        }
        ComPtr<ICLRRuntimeInfo> runtimeInfo;
        ULONG count = 0;
        result = enumerator->Next(1, (IUnknown**)&runtimeInfo, &count);
        if (FAILED(result)) {
            note::HResultToFile(TAG "Cannot enumerate on IEnum", result);
            return;
        }
        if (count == 0) {
            note::ToFile(TAG "There is no loaded clr runtime.");
            return;
        }

        ComPtr<ICLRRuntimeHost> runtimeHost;
        result = runtimeInfo->GetInterface(CLSID_CLRRuntimeHost, IID_PPV_ARGS(&runtimeHost));
        if (FAILED(result)) {
            note::HResultToFile(TAG "Cannot get ICLRRuntimeHost instance", result);
            return;
        }

        auto bootstrapDllPath = wstring(g_currentModuleDirPath) + L"/NeoLuaBootstrap.dll";
        auto scriptPath = wstring(g_currentModuleDirPath) + L"/ConfigScripts/" + g_currentConfig.ProcessName + L".lua";
        DWORD returnValue;
        result = runtimeHost->ExecuteInDefaultAppDomain(
            bootstrapDllPath.c_str(),
            L"NeoLuaBootstrap.Handlers",
            L"Initialize",
            scriptPath.c_str(),
            &returnValue
        );
        if (FAILED(result)) {
            note::HResultToFile(TAG "Failed to invoke NeoLuaBootstrap.Handlers.Initialize", result);
            return;
        }
    }
}
