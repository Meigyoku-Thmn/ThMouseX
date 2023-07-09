#pragma once
#include "framework.h"
#include "macro.h"
#include <functional>
#include <string_view>
#include <memory>

constexpr auto PROCESS_NAME_MAX_LEN = 64;
constexpr auto ADDRESS_CHAIN_MAX_LEN = 8;
constexpr auto GAME_CONFIG_MAX_LEN = 128;

struct ErrorMessage {
    DWORD code;
    LPCSTR symbolicName;
    LPCSTR description;
    LPCSTR sourceHeader;
};

enum ModulateStage {
    WhiteInc, WhiteDec, BlackInc, BlackDec,
};

struct RECTSIZE: RECT {
    LONG width() const {
        return right - left;
    }
    LONG height() const {
        return bottom - top;
    }
};

enum class PointDataType {
    None, Int, Float, Short
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

union TypedPoint {
    IntPoint    IntData;
    FloatPoint  FloatData;
};

struct AddressChain {
    int     Length;
    DWORD   Level[ADDRESS_CHAIN_MAX_LEN];
};
static_assert(sizeof(void*) == sizeof(AddressChain::Level[0]));

BEGIN_EXPORT_FLAG_ENUM(InputMethod, int)
None = 0,
DirectInput = 1 << 0,
GetKeyboardState = 1 << 1,
SendKey = 1 << 2,
END_FLAG_ENUM()

enum class ScriptingMethod {
    None, LuaJIT, NeoLua
};

struct GameConfig {
    WCHAR           ProcessName[PROCESS_NAME_MAX_LEN];
    AddressChain    Address;
    ScriptingMethod ScriptingMethodToFindAddress;
    PointDataType   PosDataType;
    FloatPoint      BasePixelOffset;
    DWORD           BaseHeight;
    FloatPoint      AspectRatio;
    InputMethod     InputMethods;
};

struct GameConfigArray {
    int         Length;
    GameConfig  Configs[GAME_CONFIG_MAX_LEN];
};

struct string_hash {
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;
    size_t operator()(const char* str) const { return hash_type{}(str); }
    size_t operator()(std::string_view str) const { return hash_type{}(str); }
    size_t operator()(std::string const& str) const { return hash_type{}(str); }
};

struct HMODULE_FREER {
    typedef HMODULE pointer;
    void operator()(HMODULE handle) const {
        if (handle != NULL)
            FreeLibrary(handle);
    }
};
typedef std::unique_ptr<HMODULE, HMODULE_FREER> ModuleHandle;

struct HWND_DESTROYER {
    typedef HWND pointer;
    void operator()(HWND hwnd) const {
        if (hwnd != NULL)
            DestroyWindow(hwnd);
    }
};
typedef std::unique_ptr<HWND, HWND_DESTROYER> WindowHandle;