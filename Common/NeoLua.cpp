#include <Windows.h>
#include <string>
#include "macro.h"
#include <iostream>
#include <wrl/client.h>
#include <metahost.h>
#include <comdef.h>
#include <cstdint>
#include <format>
#include <mutex>

#include "DataTypes.h"
#include "NeoLua.h"
#include "LuaApi.h"
#include "Log.h"
#include "Variables.h"
#include "Helper.h"
#include "Helper.Encoding.h"
#include "MinHook.h"

namespace minhook = common::minhook;
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

    static void DotNetFramework();
    static void UnityMono();
    static void UnityMono_Continue();
    static PVOID mono_jit_init_hook(PCSTR file);
    static PVOID mono_jit_init_version_hook(PCSTR root_domain_name, PCSTR runtime_version);

    static bool initialized = false;
    static wstring bootstrapDllPath;
    static mutex mtx;

    void Initialize() {
        using enum ScriptType;
        if (g_gameConfig.ScriptType != NeoLua || initialized)
            return;

        lock(mtx, { {
            bootstrapDllPath = wstring(g_currentModuleDirPath) + L"\\ThMouseX.DotNet.dll";
            auto wScriptPath = format(L"{}\\ConfigScripts\\{}.lua", g_currentModuleDirPath, g_gameConfig.ProcessName);
            auto scriptPath = encoding::ConvertToUtf8(wScriptPath);
            auto runtime = luaapi::ReadAttributeFromLuaScript(scriptPath, "Runtime");

            if (_putenv_s("ThMouseX_ModuleHandle", to_string(bcast<uintptr_t>(g_coreModule)).c_str()) != 0) {
                note::ToFile(TAG "Cannot set ThMouseX_ModuleHandle env: %s.", strerror(errno));
                return;
            }
            if (_wputenv_s(L"ThMouseX_ScriptPath", wScriptPath.c_str()) != 0) {
                note::ToFile(TAG "Cannot set ThMouseX_ScriptPath env: %s.", strerror(errno));
                return;
            }
            if (_putenv_s("ThMouseX_Runtime", runtime.c_str()) != 0) {
                note::ToFile(TAG "Cannot set ThMouseX_Runtime env: %s.", strerror(errno));
                return;
            }
            if (runtime == ".NET Framework")
                DotNetFramework();
            else if (runtime == "Unity Mono")
                UnityMono();
            else
                note::ToFile(TAG "Unknown specified runtime '%s' in %s.", runtime.c_str(), scriptPath.c_str());
        } });
    }

    static void DotNetFramework() {
        auto mscoree = GetModuleHandleW(L"mscoree.dll");
        if (!mscoree) {
            log::LastErrorToFile(TAG "Failed to load mscoree.dll");
            return;
        }

        auto _CLRCreateInstance = bcast<decltype(&CLRCreateInstance)>(GetProcAddress(mscoree, "CLRCreateInstance"));
        if (!_CLRCreateInstance) {
            log::LastErrorToFile(TAG "Failed to import mscoree.dll|CLRCreateInstance");
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
        initialized = true;
    }

    using mono_get_root_domain = PVOID(*)();
    using mono_thread_attach = PVOID(*)(PVOID domain);
    using mono_domain_assembly_open = PVOID(*)(PVOID domain, PCSTR name);
    using mono_assembly_get_image = PVOID(*)(PVOID assembly);
    using mono_class_from_name = PVOID(*)(PVOID image, PCSTR name_space, PCSTR name);
    using mono_class_get_method_from_name = PVOID(*)(PVOID klass, PCSTR name, int param_count);
    using mono_string_new = PVOID(*)(PVOID domain, PCSTR text);
    using mono_runtime_invoke = PVOID(*)(PVOID method, PVOID obj, PVOID* params, PVOID* exc);
    using mono_object_to_string = PVOID(*)(PVOID obj, PVOID* exc);
    using mono_string_to_utf8 = PSTR(*)(PVOID s);
    using mono_free = void(*)(PVOID ptr);
    using mono_jit_init = PVOID(*)(PCSTR file);
    using mono_jit_init_version = PVOID(*)(PCSTR root_domain_name, PCSTR runtime_version);

    static PCWSTR possibleMonoModuleNames[] = {
        L"mono.dll",
        L"mono-2.0-bdwgc.dll",
        L"mono-2.0-sgen.dll",
    };

    static HMODULE mono;

    static HMODULE GetMonoModule() {
        if (mono != nil)
            return mono;
        for (auto monoModuleName : possibleMonoModuleNames) {
            mono = GetModuleHandleW(monoModuleName);
            if (mono)
                break;
        }
        return mono;
    }

    decltype(&mono_jit_init_hook) ori_mono_jit_init;
    decltype(&mono_jit_init_version_hook) ori_mono_jit_init_version_hook;

    static bool hookSetup = false;
    static void UnityMono() {
        if (!GetMonoModule()) {
            log::LastErrorToFile(TAG "Failed to load the mono runtime");
            return;
        }

        TryImportAPI(mono, mono_get_root_domain, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_jit_init, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_jit_init_version, log::LastErrorToFile, TAG);

        auto rootDomain = _mono_get_root_domain();
        if (!rootDomain) {
            note::ToFile(TAG "Failed to get root domain of mono instance.");
            if (hookSetup)
                return;
            minhook::CreateHook(vector<minhook::HookConfig>{
                { _mono_jit_init, &mono_jit_init_hook, &ori_mono_jit_init, APP_NAME "_mono_jit_init" },
                { _mono_jit_init_version, &mono_jit_init_version_hook, &ori_mono_jit_init_version_hook, APP_NAME "_mono_jit_init_version" },
            });
            hookSetup = true;
            return;
        }
        UnityMono_Continue();
    }

    static PVOID mono_jit_init_hook(PCSTR file) {
        auto rs = ori_mono_jit_init(file);
        UnityMono_Continue();
        ImportAPI(mono, mono_jit_init);
        minhook::DisableHooks(vector<minhook::HookConfig> {
            { _mono_jit_init, nil, &ori_mono_jit_init, APP_NAME "_mono_jit_init" }
        });
        return rs;
    }
    static PVOID mono_jit_init_version_hook(PCSTR root_domain_name, PCSTR runtime_version) {
        auto rs = ori_mono_jit_init_version_hook(root_domain_name, runtime_version);
        UnityMono_Continue();
        ImportAPI(mono, mono_jit_init_version);
        minhook::DisableHooks(vector<minhook::HookConfig> {
            { _mono_jit_init_version, nil, &ori_mono_jit_init_version_hook, APP_NAME "_mono_jit_init" }
        });
        return rs;
    }

    static void UnityMono_Continue() {
        if (initialized)
            return;
        ImportAPI(mono, mono_get_root_domain);
        TryImportAPI(mono, mono_thread_attach, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_domain_assembly_open, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_assembly_get_image, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_class_from_name, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_class_get_method_from_name, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_string_new, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_runtime_invoke, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_object_to_string, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_string_to_utf8, log::LastErrorToFile, TAG);
        TryImportAPI(mono, mono_free, log::LastErrorToFile, TAG);

        auto rootDomain = _mono_get_root_domain();
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

        PVOID mono_exception{};
        PVOID args[]{ _mono_string_new(rootDomain, "") };
        _mono_runtime_invoke(method, nil, args, &mono_exception);
        if (mono_exception) {
            auto mono_err_string = _mono_object_to_string(mono_exception, nil);
            auto err_str = _mono_string_to_utf8(mono_err_string);
            note::ToFile(TAG "Failed to invoke ThMouseX.DotNet.Handlers.Initialize: %s", err_str);
            _mono_free(err_str);
            return;
        }
        initialized = true;
    }
}
