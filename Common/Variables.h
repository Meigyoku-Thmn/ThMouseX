#pragma once
#include "macro.h"
#include "framework.h"
#include "DataTypes.h"

#ifdef COMMON_EXPORTS
extern HMODULE  g_commonModule;
extern WCHAR    g_currentModuleDirPath[MAX_PATH + 1];
#endif

// single game config
EXPORT_VARIABLE____(GameConfig, g_currentConfig)

// global game state
EXPORT_VARIABLE____(bool, g_hookApplied)
EXPORT_VARIABLE____(HMODULE, g_targetModule)
EXPORT_VARIABLE____(HMODULE, g_coreModule)
EXPORT_VARIABLE____(HWND, g_hFocusWindow)
EXPORT_VARIABLE____(bool, g_leftMousePressed)
EXPORT_VARIABLE____(bool, g_midMousePressed)
EXPORT_VARIABLE____(bool, g_inputEnabled)
EXPORT_VARIABLE____(float, g_pixelRate)
EXPORT_VARIABLE____(FloatPoint, g_pixelOffset)

// configuration from main exe
EXPORT_VARIABLE____(GameConfigArray, gs_gameConfigArray)
EXPORT_VARIABLE____(BYTE, gs_bombButton)
EXPORT_VARIABLE____(BYTE, gs_extraButton)
EXPORT_VARIABLE____(DWORD, gs_toggleOsCursorButton)
EXPORT_VARIABLE_ARR(WCHAR, gs_textureFilePath, [MAX_PATH])
EXPORT_VARIABLE____(DWORD, gs_textureBaseHeight)

EXPORT_VARIABLE____(DWORD, gs_d3d9_CreateDevice_RVA)
EXPORT_VARIABLE____(DWORD, gs_d3d9_Reset_RVA)
EXPORT_VARIABLE____(DWORD, gs_d3d9_Present_RVA)

EXPORT_VARIABLE____(DWORD, gs_d3d8_CreateDevice_RVA)
EXPORT_VARIABLE____(DWORD, gs_d3d8_Reset_RVA)
EXPORT_VARIABLE____(DWORD, gs_d3d8_Present_RVA)

EXPORT_VARIABLE____(DWORD, gs_dinput8_GetDeviceState_RVA)