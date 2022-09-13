module;

#include "framework.h"

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

export struct AddressChain {
    int     Length;
    DWORD   Chain[ADDRESS_CHAIN_MAX_LEN];
};

export struct GameConfig {
    char            ProcessName[PROCESS_NAME_MAX_LEN];
    AddressChain    Posistion;
    PointDataType   PosDataType;
    float           PixelRate;
    FloatPoint      PixelOffset;
    FloatPoint      BasePixelOffset;
    unsigned int    BaseResolutionX;
    bool            OffsetIsRelative;
    char            BaseName[16];
};

export struct GameConfigArray {
    int         Length;
    GameConfig  Configs[GAME_CONFIG_MAX_LEN];
};
