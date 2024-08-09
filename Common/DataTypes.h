#pragma once
#include "framework.h"
#include "macro.h"
#include <functional>
#include <string_view>
#include <memory>
#include <array>
#include <type_traits>
#include <algorithm>

constexpr auto PROCESS_NAME_MAX_LEN = 64;
constexpr auto ADDRESS_CHAIN_MAX_LEN = 8;
constexpr auto GAME_CONFIG_MAX_LEN = 128;

struct ErrorMessage {
    HRESULT code;
    LPCSTR symbolicName;
    LPCSTR description;
    LPCSTR sourceHeader;
    ErrorMessage(HRESULT code, LPCSTR symbolicName, LPCSTR description, LPCSTR sourceHeader) :
        code(code), symbolicName(symbolicName), description(description), sourceHeader(sourceHeader) {
    }
};

enum class ModulateStage {
    WhiteInc, WhiteDec, BlackInc, BlackDec,
};

struct RECTSIZE : RECT {
    LONG width() const {
        return right - left;
    }
    LONG height() const {
        return bottom - top;
    }
};

enum class PointDataType {
    None, Int, Float, Short, Double
};

struct IntPoint {
    int X;
    int Y;
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

struct AddressChain {
    std::size_t Length;
    DWORD       Level[ADDRESS_CHAIN_MAX_LEN];
};
static_assert(sizeof(void*) == sizeof(AddressChain::Level[0]), "Support 32-bit system only!");

BEGIN_FLAG_ENUM(GameInput, DWORD)
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

MOVE_LEFT/*   */ = 1 << 9,
MOVE_RIGHT/*  */ = 1 << 10,
MOVE_UP/*     */ = 1 << 11,
MOVE_DOWN/*   */ = 1 << 12
END_FLAG_ENUM()

BEGIN_FLAG_ENUM(InputMethod, int)
None/*        */ = 0,
DirectInput/* */ = 1 << 0,
GetKeyboardState = 1 << 1,
SendInput/*   */ = 1 << 2,
SendMsg/*     */ = 1 << 3,
END_FLAG_ENUM()

enum class ScriptType {
    None, LuaJIT, NeoLua, Lua,
};

struct GameConfig {
    WCHAR                   ProcessName[PROCESS_NAME_MAX_LEN];
    AddressChain            Address;

    ScriptType              ScriptType;

    PointDataType           PosDataType;
    FloatPoint              BasePixelOffset;
    DWORD                   BaseHeight;
    FloatPoint              AspectRatio;
    InputMethod             InputMethods;
};

template <class ElementType, size_t ArraySize>
class _GameConfigs : public std::array<ElementType, ArraySize> {
private:
    _GameConfigs::size_type _length;
public:
    constexpr ElementType& add_new() {
        return this->at(_length++);
    }
    constexpr _GameConfigs::size_type length() const noexcept {
        return _length;
    }
    constexpr _GameConfigs::size_type capacity() const noexcept {
        return this->max_size();
    }
};
// This class must be instantiated with an initializer, preferably {}.
using GameConfigs = _GameConfigs<GameConfig, GAME_CONFIG_MAX_LEN>;
static_assert(std::is_trivial_v<GameConfigs>);

struct string_hash {
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
    size_t operator()(const char* str) const { return hash_type{}(str); }
    size_t operator()(std::string_view str) const { return hash_type{}(str); }
    size_t operator()(std::string const& str) const { return hash_type{}(str); }
};

struct HMODULE_FREER {
    using pointer = HMODULE;
    void operator()(HMODULE handle) const {
        if (handle != nil)
            FreeLibrary(handle);
    }
};
using ModuleHandle = std::unique_ptr<HMODULE, HMODULE_FREER>;

struct HWND_DESTROYER {
    using pointer = HWND;
    void operator()(HWND hwnd) const {
        if (hwnd != nil)
            DestroyWindow(hwnd);
    }
};
using WindowHandle = std::unique_ptr<HWND, HWND_DESTROYER>;

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
