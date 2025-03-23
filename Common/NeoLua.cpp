#include <Windows.h>
#include <string>
#include "macro.h"
#include <iostream>
#include <wrl/client.h>
#include <metahost.h>
#include <comdef.h>
#include <cstdint>
#include <format>

#include "DataTypes.h"
#include "NeoLua.h"
#include "LuaApi.h"
#include "Log.h"
#include "Variables.h"
#include "Helper.h"
#include "Helper.Encoding.h"

namespace note = common::log;
namespace helper = common::helper;
namespace luaapi = common::luaapi;
namespace encoding = common::helper::encoding;

using namespace std;
using namespace Microsoft::WRL;

#define TAG "[NeoLua] "

namespace common::neolua {
    uintptr_t GetPositionAddress() {
        return luaapi::GetPositionAddress();
    }

    static void DotNetFramework(const wstring& bootstrapDllPath);
    static void UnityMono(const wstring& bootstrapDllPath);

    void Initialize() {
        if (g_gameConfig.ScriptType != ScriptType_NeoLua)
            return;

        auto bootstrapDllPath = wstring(g_currentModuleDirPath) + L"/ThMouseX.DotNet.dll";
        auto wScriptPath = format(L"{}/ConfigScripts/{}.lua", g_currentModuleDirPath, g_gameConfig.processName);
        auto scriptPath = encoding::ConvertToUtf8(wScriptPath);
        auto runtime = luaapi::ReadAttributeFromLuaScript(scriptPath, "Runtime");

        if (_putenv(format("ThMouseX_ModuleHandle={}", (uintptr_t)g_coreModule).c_str()) != 0) {
            note::ToFile(TAG "Cannot set ThMouseX_ModuleHandle env.");
            return;
        }
        if (_wputenv(format(L"ThMouseX_ScriptPath={}", wScriptPath).c_str()) != 0) {
            note::ToFile(TAG "Cannot set ThMouseX_ScriptPath env.");
            return;
        }

        if (runtime == ".NET Framework")
            DotNetFramework(bootstrapDllPath);
        else if (runtime == "Unity Mono")
            UnityMono(bootstrapDllPath);
        else
            note::ToFile(TAG " Unknown specified runtime '%s' in %s.", runtime.c_str(), scriptPath.c_str());
    }

    static void DotNetFramework(const wstring& bootstrapDllPath) {
#pragma region Preparation
        auto mscoree = GetModuleHandleW(L"mscoree.dll");
        if (!mscoree) {
            log::LastErrorToFile(TAG " Failed to load mscoree.dll");
            return;
        }

        auto _CLRCreateInstance = bcast<decltype(&CLRCreateInstance)>(GetProcAddress(mscoree, "CLRCreateInstance"));
        if (!_CLRCreateInstance) {
            log::LastErrorToFile(TAG " Failed to import mscoree.dll|CLRCreateInstance");
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
            if (helper::Win32FromHResult(result) != ERROR_BAD_LENGTH)
                break;
        }
        if (FAILED(result)) {
            note::HResultToFile(TAG "Cannot enumerate loaded clr runtimes", result); return;
        }
        ComPtr<ICLRRuntimeInfo> runtimeInfo;
        ULONG count = 0;
        result = enumerator->Next(1, scast<IUnknown**>(&runtimeInfo), &count);
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
#pragma endregion

        DWORD returnValue;
        result = runtimeHost->ExecuteInDefaultAppDomain(
            bootstrapDllPath.c_str(),
            L"ThMouseX.DotNet.Handlers",
            L"Initialize",
            L"",
            &returnValue
        );
        if (FAILED(result)) {
            note::HResultToFile(TAG "Failed to invoke ThMouseX.DotNet.Handlers.Initialize", result);
            return;
        }
    }

    using mono_get_root_domain = PVOID(*)();
    using mono_thread_attach = PVOID(*)(PVOID domain);
    using mono_domain_assembly_open = PVOID(*)(PVOID domain, PCSTR name);
    using mono_assembly_get_image = PVOID(*)(PVOID assembly);
    using mono_class_from_name = PVOID(*)(PVOID image, PCSTR name_space, PCSTR name);
    using mono_class_get_method_from_name = PVOID(*)(PVOID klass, PCSTR name, int param_count);
    using mono_runtime_invoke = PVOID(*)(PVOID method, PVOID obj, PVOID* params, PVOID* exc);
    using mono_object_to_string = PVOID(*)(PVOID obj, PVOID* exc);
    using mono_string_chars = PCWSTR(*)(PVOID s);

    static void UnityMono(const wstring& bootstrapDllPath) {
#pragma region Preparation
        auto mono = GetModuleHandleW(L"mono.dll");
        if (!mono)
            mono = GetModuleHandleW(L"mono-2.0-bdwgc.dll");
        if (!mono)
            mono = GetModuleHandleW(L"mono-2.0-sgen.dll");
        if (!mono) {
            log::LastErrorToFile(TAG " Failed to load the mono runtime");
            return;
        }

        ImportAPI(mono, mono_get_root_domain);
        if (!_mono_get_root_domain) {
            log::LastErrorToFile(TAG " Failed to import mono_get_root_domain");
            return;
        }
        ImportAPI(mono, mono_thread_attach);
        if (!_mono_thread_attach) {
            log::LastErrorToFile(TAG " Failed to import mono_thread_attach");
            return;
        }
        ImportAPI(mono, mono_domain_assembly_open);
        if (!_mono_domain_assembly_open) {
            log::LastErrorToFile(TAG " Failed to import mono_domain_assembly_open");
            return;
        }
        ImportAPI(mono, mono_assembly_get_image);
        if (!_mono_assembly_get_image) {
            log::LastErrorToFile(TAG " Failed to import mono_assembly_get_image");
            return;
        }
        ImportAPI(mono, mono_class_from_name);
        if (!_mono_class_from_name) {
            log::LastErrorToFile(TAG " Failed to import mono_class_from_name");
            return;
        }
        ImportAPI(mono, mono_class_get_method_from_name);
        if (!_mono_class_get_method_from_name) {
            log::LastErrorToFile(TAG " Failed to import mono_class_get_method_from_name");
            return;
        }
        ImportAPI(mono, mono_runtime_invoke);
        if (!_mono_runtime_invoke) {
            log::LastErrorToFile(TAG " Failed to import mono_runtime_invoke");
            return;
        }
        ImportAPI(mono, mono_object_to_string);
        if (!_mono_object_to_string) {
            log::LastErrorToFile(TAG " Failed to import mono_object_to_string");
            return;
        }
        ImportAPI(mono, mono_string_chars);
        if (!_mono_string_chars) {
            log::LastErrorToFile(TAG " Failed to import mono_string_chars");
            return;
        }

        auto rootDomain = _mono_get_root_domain();
        if (!rootDomain) {
            note::ToFile(TAG "Failed to get root domain of mono instance.");
            return;
        }
        auto monoThread = _mono_thread_attach(rootDomain);
        if (!monoThread) {
            note::ToFile(TAG "Failed to register the current thread to mono instance.");
            return;
        }
        auto assembly = _mono_domain_assembly_open(rootDomain, encoding::ConvertToUtf8(bootstrapDllPath).c_str());
        if (!assembly) {
            note::ToFile(TAG "Failed to load ThMouseX.DotNet.dll into mono instance.");
            return;
        }
        auto image = _mono_assembly_get_image(assembly);
        if (!image) {
            note::ToFile(TAG "Failed to obtain image of the assembly ThMouseX.DotNet.dll.");
            return;
        }
        auto klass = _mono_class_from_name(image, "ThMouseX.DotNet", "Handlers");
        if (!klass) {
            note::ToFile(TAG "Failed to get the class ThMouseX.DotNet.Handlers.");
            return;
        }
        auto method = _mono_class_get_method_from_name(klass, "Initialize", 1);
        if (!method) {
            note::ToFile(TAG "Failed to get the method ThMouseX.DotNet.Handlers.Initialize.");
            return;
        }
#pragma endregion
        PVOID mono_exception{};
        _mono_runtime_invoke(method, nullptr, nullptr, &mono_exception);
        if (mono_exception) {
            auto mono_err_string = _mono_object_to_string(mono_exception, nil);
            auto err_str = _mono_string_chars(mono_err_string);
            note::ToFile(TAG "Failed to invoke ThMouseX.DotNet.Handlers.Initialize: %s", err_str);
            return;
        }
    }
}
