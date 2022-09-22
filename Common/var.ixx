module;

#include "macro.h"
#include "framework.h"

export module common.var;

import common.datatype;

// single game config
export DLLEXPORT GameConfig       g_currentConfig;

// global game state
export DLLEXPORT HWND             g_hFocusWindow;
export DLLEXPORT bool             g_leftMousePressed;
export DLLEXPORT bool             g_midMousePressed;
export DLLEXPORT bool             g_inputEnabled;
export DLLEXPORT float            g_pixelRate = 1;
export DLLEXPORT FloatPoint       g_pixelOffset{1, 1};

export DLLEXPORT bool             g_handledByDirectInput;
export DLLEXPORT bool             g_handledByGetKeyboardState;
export DLLEXPORT bool             g_handledByJoyGetPosEx;

// configuration from main exe
#pragma data_seg(".SHRCONF")
export DLLEXPORT GameConfigArray  gs_gameConfigArray{};
export DLLEXPORT DWORD            gs_boomButton = 1;
export DLLEXPORT DWORD            gs_extraButton = 3;
export DLLEXPORT WCHAR            gs_textureFilePath[MAX_PATH]{};
export DLLEXPORT DWORD            gs_textureBaseHeight = 480;

export DLLEXPORT DWORD            gs_d3d9_Reset_RVA{};
export DLLEXPORT DWORD            gs_d3d9_Present_RVA{};

export DLLEXPORT DWORD            gs_d3d8_Reset_RVA{};
export DLLEXPORT DWORD            gs_d3d8_Present_RVA{};

export DLLEXPORT DWORD            gs_dinput8_GetDeviceState_RVA{};
#pragma data_seg()
// make the above segment shared across processes
#pragma comment(linker, "/SECTION:.SHRCONF,RWS")