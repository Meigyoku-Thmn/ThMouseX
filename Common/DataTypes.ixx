module;

#include "framework.h"
#include "macro.h"
#include <functional>
#include <string_view>

export module common.datatype;

using namespace std;

constexpr auto PROCESS_NAME_MAX_LEN = 64;
constexpr auto ADDRESS_CHAIN_MAX_LEN = 8;
constexpr auto GAME_CONFIG_MAX_LEN = 128;

export enum ModulateStage {
    WhiteInc, WhiteDec, BlackInc, BlackDec,
};

export struct RECTSIZE: RECT {
    UNBOUND inline LONG width() const {
        return right - left;
    }
    UNBOUND inline LONG height() const {
        return bottom - top;
    }
};

export enum class PointDataType {
    None, Int, Float, Short
};

export struct IntPoint {
    int X;
    int Y;
};

export struct ShortPoint {
    short X;
    short Y;
};

export struct FloatPoint {
    float X;
    float Y;
};

export union TypedPoint {
    IntPoint    IntData;
    FloatPoint  FloatData;
};

export struct AddressChain {
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

export enum class ScriptingMethod {
    None, LuaJIT, NeoLua
};

export struct GameConfig {
    WCHAR           ProcessName[PROCESS_NAME_MAX_LEN];
    AddressChain    Address;
    ScriptingMethod ScriptingMethodToFindAddress;
    PointDataType   PosDataType;
    FloatPoint      BasePixelOffset;
    DWORD           BaseHeight;
    FloatPoint      AspectRatio;
    InputMethod     InputMethods;
};

export struct GameConfigArray {
    int         Length;
    GameConfig  Configs[GAME_CONFIG_MAX_LEN];
};

export struct string_hash {
    using hash_type = hash<string_view>;
    using is_transparent = void;
    UNBOUND size_t operator()(const char* str) const { return hash_type{}(str); }
    UNBOUND size_t operator()(string_view str) const { return hash_type{}(str); }
    UNBOUND size_t operator()(string const& str) const { return hash_type{}(str); }
};