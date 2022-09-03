#include "subGlobal.h"
#include "stdafx.h"

HMODULE _hModule = NULL;
HWND g_hFocusWindow2 = 0;
BOOL g_working2 = 0;
BOOL windowed2 = FALSE;
char* gs_textureFilePath2 = NULL;
FLOAT g_currentScale2 = 1.0f;
FLOAT* pixelRate;
FloatPoint2* pixelOffset;
FloatPoint2* basePixelOffset;
UINT baseResolutionX;