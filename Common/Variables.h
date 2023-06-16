#pragma once
#include "macro.h"
#include "framework.h"
#include "DataTypes.h"

extern HMODULE  g_commonModule;
extern WCHAR    g_currentModuleDirPath[MAX_PATH + 1];

// single game config
DLLEXPORT extern GameConfig g_currentConfig;

// global game state
DLLEXPORT extern bool       g_hookApplied;
DLLEXPORT extern HMODULE    g_targetModule;
DLLEXPORT extern HMODULE    g_coreModule;
DLLEXPORT extern HWND       g_hFocusWindow;
DLLEXPORT extern bool       g_leftMousePressed;
DLLEXPORT extern bool       g_midMousePressed;
DLLEXPORT extern bool       g_inputEnabled;
DLLEXPORT extern float      g_pixelRate;
DLLEXPORT extern FloatPoint g_pixelOffset;

// configuration from main exe
DLLEXPORT extern GameConfigArray    gs_gameConfigArray;
DLLEXPORT extern BYTE               gs_bombButton;
DLLEXPORT extern BYTE               gs_extraButton;
DLLEXPORT extern DWORD              gs_toggleOsCursorButton;
DLLEXPORT extern WCHAR              gs_textureFilePath[MAX_PATH];
DLLEXPORT extern DWORD              gs_textureBaseHeight;

DLLEXPORT extern DWORD              gs_d3d9_CreateDevice_RVA;
DLLEXPORT extern DWORD              gs_d3d9_Reset_RVA;
DLLEXPORT extern DWORD              gs_d3d9_Present_RVA;

DLLEXPORT extern DWORD              gs_d3d8_CreateDevice_RVA;
DLLEXPORT extern DWORD              gs_d3d8_Reset_RVA;
DLLEXPORT extern DWORD              gs_d3d8_Present_RVA;

DLLEXPORT extern DWORD              gs_dinput8_GetDeviceState_RVA;