#pragma once
#include "framework.h"
#include "macro.h"

#ifndef DX8HOOK_EXPORTS
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <string>
namespace dx8::dll {
    using namespace std;
    inline WCHAR currentModuleDirPath[MAX_PATH + 1];
    inline HMODULE hMod;
    inline FARPROC ImportFunction(const char* functionName) {
        if (hMod == NULL) {
            auto modulePath = wstring(currentModuleDirPath) + L"\\" + L"DX8Hook.dll";
            hMod = LoadLibraryW(modulePath.c_str());
        }
        if (hMod == NULL)
            return NULL;
        return GetProcAddress(hMod, functionName);
    }
    inline BOOL Unload() {
        auto rs = FreeLibrary(hMod);
        hMod = NULL;
        return rs;
    }
    inline struct OnInit_RunTimeDllLoad {
        OnInit_RunTimeDllLoad() {
            HMODULE hmod;
            auto dwFlags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
            GetModuleHandleExW(dwFlags, (LPWSTR)&ImportFunction, &hmod);
            GetModuleFileNameW(hmod, currentModuleDirPath, ARRAYSIZE(currentModuleDirPath));
            currentModuleDirPath[ARRAYSIZE(currentModuleDirPath) - 1] = '\0';
            PathRemoveFileSpecW(currentModuleDirPath);
        }
        ~OnInit_RunTimeDllLoad() {
            Unload();
        }
    } _;
}
#endif

#ifdef DX8HOOK_EXPORTS
#define RUNTIME_EXPORT_FUNC(retType, funcName, ...) \
retType funcName(__VA_ARGS__); \
DLLEXPORT_C decltype(&funcName) GetFunc_##funcName() { \
    return &funcName; \
}
#else
#define RUNTIME_EXPORT_FUNC(retType, funcName, ...) \
retType funcName##_Sig(__VA_ARGS__); \
inline decltype(&funcName##_Sig) funcName; \
decltype(&funcName##_Sig) GetFunc_##funcName(); \
inline struct OnInit_##funcName##_Func { \
    OnInit_##funcName##_Func() { \
        auto getter = (decltype(&GetFunc_##funcName))dx8::dll::ImportFunction(QUOTE(GetFunc_##funcName)); \
        funcName = getter(); \
    } \
} _init_var_##funcName;
#endif