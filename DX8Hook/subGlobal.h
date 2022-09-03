#pragma once

#include <Windows.h>
#include "ExportMacro.h"

typedef struct tagFloatPoint2 {
    float X;
    float Y;
} FloatPoint2;

DLLEXPORT extern HMODULE _hModule;
DLLEXPORT extern HWND g_hFocusWindow2;
DLLEXPORT extern BOOL windowed2;
DLLEXPORT extern char* gs_textureFilePath2;
DLLEXPORT extern int g_working2;

DLLEXPORT extern FLOAT g_currentScale2;
DLLEXPORT extern FLOAT g_currentScale2;
DLLEXPORT extern FLOAT* pixelRate;
DLLEXPORT extern FloatPoint2* pixelOffset;
DLLEXPORT extern FloatPoint2* basePixelOffset;
DLLEXPORT extern UINT baseResolutionX;
