module;

#include "macro.h"
#include "framework.h"

export module common.var;

import common.datatype;

export DLLEXPORT HWND             g_hFocusWindow;
export DLLEXPORT bool             g_mouseDown;
export DLLEXPORT bool             g_midMouseDown;
export DLLEXPORT GameConfig       g_currentGameConfig;
export DLLEXPORT DWORD            g_boomButton;
export DLLEXPORT DWORD            g_extraButton;
export DLLEXPORT bool             g_working;
export DLLEXPORT bool             g_handledByDirectInput;
export DLLEXPORT bool             g_windowed = true;
export DLLEXPORT float            g_currentScale;
export DLLEXPORT bool             g_useAccurateMousePosition = false;
export DLLEXPORT int              g_dxVersion = 9;
export DLLEXPORT bool             g_offsetIsRelative = false;
export DLLEXPORT int              g_baseOfCode;
export DLLEXPORT unsigned long    g_firstOffsetDirection;
export DLLEXPORT HMODULE          g_hModule;
export DLLEXPORT HWND             g_hFocusWindow2;
export DLLEXPORT bool             g_working2;
export DLLEXPORT bool             g_windowed2;
export DLLEXPORT char             *gs_textureFilePath2;
export DLLEXPORT float            g_currentScale2;
export DLLEXPORT float            *g_pixelRate;
export DLLEXPORT FloatPoint       *g_pixelOffset;
export DLLEXPORT FloatPoint       *g_basePixelOffset;
export DLLEXPORT UINT             g_baseResolutionX;

// configuration from main exe
#pragma data_seg(".CONFIG")
export DLLEXPORT GameConfigArray  gs_gameConfigArray{};
export DLLEXPORT DWORD            gs_boomButton{};
export DLLEXPORT DWORD            gs_extraButton{};
export DLLEXPORT char             gs_textureFilePath[TEXTURE_FILE_PATH_LEN]{};

export DLLEXPORT DWORD            gs_d3d9_Reset_RVA{};
export DLLEXPORT DWORD            gs_d3d9_EndScene_RVA{};

export DLLEXPORT DWORD            gs_d3d8_Reset_RVA{};
export DLLEXPORT DWORD            gs_d3d8_EndScene_RVA{};

export DLLEXPORT DWORD            gs_dinput8_GetDeviceState_RVA{};

export DLLEXPORT DWORD            gs_dinputw_GetDeviceState_RVA{};
#pragma data_seg()
// make the above segment shared across processes
#pragma comment(linker, "/SECTION:.CONFIG,RWS")