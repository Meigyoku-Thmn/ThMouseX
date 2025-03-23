#pragma once
#include <Windows.h>
#include "macro.h"
#include <functional>
#include <string_view>
#include <string>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <atlsafe.h>

#include "ComServer.h"

DEFINE_ENUM_FLAG_OPERATORS(InputMethod);

struct GameConfigLocal : GameConfig {
    CComSafeArray<DWORD> AddressChain;
    CComHeapPtr<WCHAR> ProcessName;
    void Initialize() {
        if (this->Address)
            this->AddressChain.Attach(this->Address);
        if (this->processName)
            this->ProcessName.Attach(this->processName);
    }
};

struct GameConfigEx : GameConfig {
    bool CopyFrom(const GameConfig& gameConfig) {
        *scast<GameConfig*>(this) = gameConfig;
        auto hr = SafeArrayCopy(this->Address, &this->Address);
        auto processNameSize = (wcslen(this->processName) + 1) * sizeof(this->processName[0]);
        auto allocated = CoTaskMemAlloc(processNameSize);
        if (allocated) {
            memcpy(allocated, this->processName, processNameSize);
            this->processName = (decltype(this->processName))allocated;
        }
        if (!allocated || FAILED(hr)) {
            SafeArrayDestroy(this->Address);
            CoTaskMemFree(allocated);
            *scast<GameConfig*>(this) = {};
            return false;
        }
        return true;
    }
};

struct CommonConfigLocal : CommonConfig {
    CComHeapPtr<WCHAR> _TextureFilePath;
    CComHeapPtr<WCHAR> _ImGuiFontPath;
    void Initialize() {
        if (this->TextureFilePath)
            this->_TextureFilePath.Attach(this->TextureFilePath);
        if (this->ImGuiFontPath)
            this->_ImGuiFontPath.Attach(this->ImGuiFontPath);
    }
};

struct CommonConfigEx : CommonConfig {
    bool CopyFrom(const CommonConfig& commonConfig) {
        *scast<CommonConfig*>(this) = commonConfig;
        auto textureFilePathSize = (wcslen(this->TextureFilePath) + 1) * sizeof(this->TextureFilePath[0]);
        auto allocated1 = CoTaskMemAlloc(textureFilePathSize);
        if (allocated1) {
            memcpy(allocated1, this->TextureFilePath, textureFilePathSize);
            this->TextureFilePath = (decltype(this->TextureFilePath))allocated1;
        }
        auto imGuiFontPathSize = (wcslen(this->ImGuiFontPath) + 1) * sizeof(this->ImGuiFontPath[0]);
        auto allocated2 = CoTaskMemAlloc(imGuiFontPathSize);
        if (allocated2) {
            memcpy(allocated2, this->ImGuiFontPath, imGuiFontPathSize);
            this->ImGuiFontPath = (decltype(this->ImGuiFontPath))allocated2;
        }
        if (!allocated1 || !allocated2) {
            CoTaskMemFree(allocated1);
            CoTaskMemFree(allocated2);
            *scast<CommonConfig*>(this) = {};
            return false;
        }
        return true;
    }
};

struct ErrorMessage {
    HRESULT code;
    LPCSTR symbolicName;
    LPCSTR description;
    LPCSTR sourceHeader;
    ErrorMessage(HRESULT code, LPCSTR symbolicName, LPCSTR description, LPCSTR sourceHeader) :
        code(code), symbolicName(symbolicName), description(description), sourceHeader(sourceHeader) {
    }
    ErrorMessage(UINT code, LPCSTR symbolicName, LPCSTR description, LPCSTR sourceHeader) :
        code((HRESULT)code), symbolicName(symbolicName), description(description), sourceHeader(sourceHeader) {
    }
};

enum class ModulateStage {
    WhiteInc, WhiteDec, BlackInc, BlackDec,
};

enum class MovementAlgorithm : int {
    Bresenham, Simple
};

struct RECTSIZE : RECT {
    LONG width() const {
        return right - left;
    }
    LONG height() const {
        return bottom - top;
    }
};

enum class GameInput : DWORD {
    NONE/*          */ = 0,

    CLICK_LEFT/*    */ = 1 << 0,
    CLICK_MIDDLE/*  */ = 1 << 1,
    CLICK_RIGHT/*   */ = 1 << 2,
    CLICK_XBUTTON1/**/ = 1 << 3,
    CLICK_XBUTTON2/**/ = 1 << 4,

    SCROLL_UP/*     */ = 1 << 5,
    SCROLL_DOWN/*   */ = 1 << 6,
    SCROLL_LEFT/*   */ = 1 << 7,
    SCROLL_RIGHT/*  */ = 1 << 8,

    MOVE_LEFT/*     */ = 1 << 9,
    MOVE_RIGHT/*    */ = 1 << 10,
    MOVE_UP/*       */ = 1 << 11,
    MOVE_DOWN/*     */ = 1 << 12
};
DEFINE_ENUM_FLAG_OPERATORS(GameInput);

struct string_hash {
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
    size_t operator()(PCSTR str) const { return hash_type{}(str); }
    size_t operator()(std::string_view str) const { return hash_type{}(str); }
    size_t operator()(std::string const& str) const { return hash_type{}(str); }
};

struct HANDLE_CLOSER {
    using pointer = HANDLE;
    void operator()(pointer handle) const {
        if (handle != nil && handle != INVALID_HANDLE_VALUE)
            CloseHandle(handle);
    }
};
using Handle = std::unique_ptr<HANDLE, HANDLE_CLOSER>;

struct HModuleDeleter {
    using pointer = HMODULE;
    void operator()(pointer hModule) const {
        if (hModule != nil)
            FreeLibrary(hModule);
    }
};
using ModuleHandle = std::unique_ptr<HMODULE, HModuleDeleter>;

struct HwndDeleter {
    using pointer = HWND;
    void operator()(pointer hwnd) const {
        if (hwnd != nil)
            DestroyWindow(hwnd);
    }
};
using WindowHandle = std::unique_ptr<HWND, HwndDeleter>;

struct TimerQueueTimerHandleDeleter {
    using pointer = HANDLE;
    void operator()(pointer handle) const {
        if (handle != nil) {
            auto _ = DeleteTimerQueueTimer(nil, handle, nil);
            if (_ == FALSE) {
                // ignore
            }
        }
    }
};
using TimerQueueTimerHandle = std::unique_ptr<HANDLE, TimerQueueTimerHandleDeleter>;

struct ActCtxCookieDeleter {
    using pointer = ULONG_PTR;
    void operator()(pointer cookie) const {
        if (cookie != 0)
            DeactivateActCtx(0, cookie);
    }
};
using ActCtxCookie = std::unique_ptr<ULONG_PTR, ActCtxCookieDeleter>;

// https://dev.to/sgf4/strings-as-template-parameters-c20-4joh
template<std::size_t N>
struct CompileTimeString {
    char data[N]{};

    explicit(false) consteval CompileTimeString(const char(&str)[N]) {
        std::copy_n(str, N, data);
    }

    consteval bool operator==(const CompileTimeString<N> str) const {
        return std::equal(str.data, str.data + N, data);
    }

    template<std::size_t N2>
    consteval bool operator==(UNUSED const CompileTimeString<N2> s) const {
        return false;
    }

    template<std::size_t N2>
    consteval CompileTimeString<N + N2 - 1> operator+(const CompileTimeString<N2> str) const {
        char newchar[N + N2 - 1]{};
        std::copy_n(data, N - 1, newchar);
        std::copy_n(str.data, N2, newchar + N - 1);
        return newchar;
    }

    consteval char operator[](std::size_t n) const {
        return data[n];
    }

    consteval std::size_t size() const {
        return N - 1;
    }
};

template<std::size_t s1, std::size_t s2>
consteval auto operator+(CompileTimeString<s1> fs, const char(&str)[s2]) {
    return fs + CompileTimeString<s2>(str);
}

template<std::size_t s1, std::size_t s2>
consteval auto operator+(const char(&str)[s2], CompileTimeString<s1> fs) {
    return CompileTimeString<s2>(str) + fs;
}

template<std::size_t s1, std::size_t s2>
consteval auto operator==(CompileTimeString<s1> fs, const char(&str)[s2]) {
    return fs == CompileTimeString<s2>(str);
}

template<std::size_t s1, std::size_t s2>
consteval auto operator==(const char(&str)[s2], CompileTimeString<s1> fs) {
    return CompileTimeString<s2>(str) == fs;
}

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

using UNICODE_STRING = struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PVOID Buffer;
};
using PUNICODE_STRING = UNICODE_STRING*;

using ANSI_STRING = struct _ANSI_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
};
using PANSI_STRING = ANSI_STRING*;

using RtlInitUnicodeString = VOID(NTAPI*)(PUNICODE_STRING DestinationString, PCWSTR SourceString);
using RtlInitAnsiString = VOID(NTAPI*)(PANSI_STRING DestinationString, PCSTR SourceString);
using LdrLoadDll = NTSTATUS(NTAPI*)(PWCHAR PathToFile, ULONG Flags, PUNICODE_STRING ModuleFileName, HMODULE* ModuleHandle);
using LdrGetProcedureAddress = NTSTATUS(NTAPI*)(HMODULE ModuleHandle, PANSI_STRING FunctionName, WORD Oridinal, PVOID* FunctionAddress);
using LdrUnloadDll = NTSTATUS(NTAPI*)(HMODULE ModuleHandle);

struct ShellcodeInput {
    FixedStringMember(WCHAR, user32dll, L"user32.dll");
    FixedStringMember(CHAR, peekMessageW, "PeekMessageW");
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    ImportAPI(ntdll, RtlInitUnicodeString);
    ImportAPI(ntdll, RtlInitAnsiString);
    ImportAPI(ntdll, LdrLoadDll);
    ImportAPI(ntdll, LdrGetProcedureAddress);
    ImportAPI(ntdll, LdrUnloadDll);
};

using ThreadFunc = LPTHREAD_START_ROUTINE;

using UninitializeCallbackType = void(__cdecl*)(bool isProcessTerminating);
using CallbackType = void(__cdecl*)();