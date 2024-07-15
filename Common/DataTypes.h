#pragma once
#include "framework.h"
#include "macro.h"
#include <functional>
#include <string_view>
#include <memory>
#include <array>
#include <type_traits>

constexpr auto PROCESS_NAME_MAX_LEN = 64;
constexpr auto ADDRESS_CHAIN_MAX_LEN = 8;
constexpr auto GAME_CONFIG_MAX_LEN = 128;

struct ErrorMessage {
    DWORD code;
    LPCSTR symbolicName;
    LPCSTR description;
    LPCSTR sourceHeader;
    ErrorMessage(unsigned int code, const char* symbolicName, const char* description, const char* sourceHeader) :
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
static_assert(sizeof(void*) == sizeof(AddressChain::Level[0]));

BEGIN_FLAG_ENUM(GameInput, DWORD)
NONE/*        */ = 0,
USE_BOMB/*    */ = 0b0000'0001,
USE_SPECIAL/* */ = 0b0000'0010,

MOVE_LEFT/*   */ = 0b1000'0000,
MOVE_RIGHT/*  */ = 0b0100'0000,
MOVE_UP/*     */ = 0b0010'0000,
MOVE_DOWN/*   */ = 0b0001'0000,
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
        if (handle != nullptr)
            FreeLibrary(handle);
    }
};
using ModuleHandle = std::unique_ptr<HMODULE, HMODULE_FREER>;

struct HWND_DESTROYER {
    using pointer = HWND;
    void operator()(HWND hwnd) const {
        if (hwnd != nullptr)
            DestroyWindow(hwnd);
    }
};
using WindowHandle = std::unique_ptr<HWND, HWND_DESTROYER>;