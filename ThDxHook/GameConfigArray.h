#pragma once

#define PROCESS_NAME_MAX_LEN 64
#define ADDRESS_CHAIN_MAX_LEN 8
#define GAME_CONFIG_MAX_LEN 128

enum PointDataType {
    Int_DataType, Float_DataType, Short_DataType
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
    IntPoint IntData;
    FloatPoint FloatData;
};

struct AddressChain {
    int Length;
    DWORD Chain[ADDRESS_CHAIN_MAX_LEN];
};

struct GameConfig {
    char ProcessName[PROCESS_NAME_MAX_LEN];
    AddressChain Posistion;
    PointDataType PosDataType;
    float PixelRate;
    FloatPoint PixelOffset;
    FloatPoint BasePixelOffset;
    UINT BaseResolutionX;
    bool OffsetIsRelative;
    char BaseName[16];
};

struct GameConfigArray {
    int Length;
    GameConfig Configs[GAME_CONFIG_MAX_LEN];
};
