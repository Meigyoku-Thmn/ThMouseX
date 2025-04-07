#pragma once
#include <Windows.h>
#include "macro.h"
#include <functional>
#include <string_view>
#include <string>
#include <memory>
#include <type_traits>
#include <algorithm>

enum class InputMethod : int {
    None/*........*/ = 0,
    DirectInput/*.*/ = 1 << 0,
    GetKeyboardState = 1 << 1,
    SendInput/*...*/ = 1 << 2,
    SendMsg/*.....*/ = 1 << 3
};
DEFINE_ENUM_FLAG_OPERATORS(InputMethod);

enum class ScriptType : int {
    None = 0,
    LuaJIT = 1,
    NeoLua = 2,
    Lua = 3
};

enum class PointDataType : int {
    None = 0,
    Int = 1,
    Float = 2,
    Short = 3,
    Double = 4
};

struct IntPoint {
    long X;
    long Y;
};

struct ShortPoint {
    short X;
    short Y;
};

struct FloatPoint {
    float X;
    float Y;
};

struct DoublePoint {
    double X;
    double Y;
};

struct CommonConfig {
    BYTE VkCodeForLeftClick;
    BYTE VkCodeForMiddleClick;
    BYTE VkCodeForRightClick;
    BYTE VkCodeForXButton1Click;
    BYTE VkCodeForXButton2Click;
    BYTE VkCodeForScrollUp;
    BYTE VkCodeForScrollDown;
    BYTE VkCodeForScrollLeft;
    BYTE VkCodeForScrollRight;
    BYTE ToggleMouseControl;
    BYTE ToggleOsCursorButton;
    BYTE ToggleImGuiButton;
    LPWSTR TextureFilePath;
    ULONG TextureBaseHeight;
    LPWSTR ImGuiFontPath;
    ULONG ImGuiBaseFontSize;
    ULONG ImGuiBaseVerticalResolution;
};


struct GameConfig {
    LPWSTR processName;
    DWORD NumOfOffsets;
    PDWORD Offsets;
    ScriptType ScriptType;
    PointDataType PosDataType;
    FloatPoint BasePixelOffset;
    ULONG BaseHeight;
    FloatPoint AspectRatio;
    InputMethod InputMethods;
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

using RtlInitUnicodeString = VOID(NTAPI*)(_Out_ PUNICODE_STRING DestinationString, _In_opt_ PCWSTR SourceString);
using RtlInitAnsiString = VOID(NTAPI*)(_Out_ PANSI_STRING DestinationString, _In_opt_ PCSTR SourceString);
using LdrLoadDll = NTSTATUS(NTAPI*)(_In_opt_ PWCHAR PathToFile, _In_ ULONG Flags, _In_ PUNICODE_STRING ModuleFileName, _Out_ HMODULE* ModuleHandle);
using LdrGetProcedureAddress = NTSTATUS(NTAPI*)(_In_ HMODULE ModuleHandle, _In_ PANSI_STRING FunctionName, _In_ WORD Oridinal, _Out_ PVOID* FunctionAddress);
using LdrUnloadDll = NTSTATUS(NTAPI*)(_In_ HMODULE ModuleHandle);

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

using UninitializeCallbackType = void(*)(bool isProcessTerminating);
using CallbackType = void(*)();