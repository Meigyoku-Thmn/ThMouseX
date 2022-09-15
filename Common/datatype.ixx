module;

#include "framework.h"
#include "macro.h"

export module common.datatype;

export constexpr auto PROCESS_NAME_MAX_LEN = 64;
export constexpr auto ADDRESS_CHAIN_MAX_LEN = 8;
export constexpr auto GAME_CONFIG_MAX_LEN = 128;
export constexpr auto TEXTURE_FILE_PATH_LEN = 256;

export struct UINTSIZE {
    unsigned int width;
    unsigned int height;
};

export enum PointDataType {
    Int_DataType, Float_DataType, Short_DataType,
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

export struct DLLEXPORT AddressChain {
    int     Length;
    DWORD   Level[ADDRESS_CHAIN_MAX_LEN];
    DWORD   value() {
        auto address = Level[0];
        for (int i = 1; i < Length; i++) {
            address = *(DWORD*)address;
            if (address == NULL)
                break;
            address += Level[i];
        }
        return address;
    }
};

export struct GameConfig {
    char            ProcessName[PROCESS_NAME_MAX_LEN];
    AddressChain    Address;
    PointDataType   PosDataType;
    FloatPoint      BasePixelOffset;
    unsigned int    BaseResolutionX;
};

export struct GameConfigArray {
    int         Length;
    GameConfig  Configs[GAME_CONFIG_MAX_LEN];
};
