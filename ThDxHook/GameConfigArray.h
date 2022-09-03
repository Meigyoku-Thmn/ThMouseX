#pragma once

#define PROCESS_NAME_MAX_LEN 64
#define ADDRESS_CHAIN_MAX_LEN 8
#define GAME_CONFIG_MAX_LEN 128

enum PointDataType {
    Int_DataType, Float_DataType, Short_DataType
};

typedef struct tagIntPoint {
    int X;
    int Y;
} IntPoint;

typedef struct tagShortPoint {
    short X;
    short Y;
} ShortPoint;

typedef struct tagFloatPoint {
    float X;
    float Y;
} FloatPoint;

typedef union tagTypedPoint {
    IntPoint IntData;
    FloatPoint FloatData;
} TypedPoint;

typedef struct tagAddressChain {
    int Length;
    DWORD Chain[ADDRESS_CHAIN_MAX_LEN];
} AddressChain;

typedef struct tagGameConfig {
    char ProcessName[PROCESS_NAME_MAX_LEN];
    AddressChain Posistion;
    PointDataType PosDataType;
    float PixelRate;
    FloatPoint PixelOffset;
    FloatPoint BasePixelOffset;
    UINT BaseResolutionX;
    bool OffsetIsRelative;
    char BaseName[16];
} GameConfig;

typedef struct tagGameConfigArray {
    int Length;
    GameConfig Configs[GAME_CONFIG_MAX_LEN];
} GameConfigArray;
