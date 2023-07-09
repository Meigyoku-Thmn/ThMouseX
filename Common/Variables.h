#pragma once
#include "macro.h"
#include "framework.h"
#include "DataTypes.h"

#include "DllLoad.h"

#ifdef COMMON_EXPORTS
extern HMODULE  g_commonModule;
extern WCHAR    g_currentModuleDirPath[MAX_PATH + 1];
#endif

// single game config
EXPORT_VAR(GameConfig, g_currentConfig)

// global game state
EXPORT_VAR(bool, g_hookApplied)
EXPORT_VAR(HMODULE, g_targetModule)
EXPORT_VAR(HMODULE, g_coreModule)
EXPORT_VAR(HWND, g_hFocusWindow)
EXPORT_VAR(bool, g_leftMousePressed)
EXPORT_VAR(bool, g_midMousePressed)
EXPORT_VAR(bool, g_inputEnabled)
EXPORT_VAR(float, g_pixelRate)
EXPORT_VAR(FloatPoint, g_pixelOffset)
EXPORT_VAR(bool, g_showImGui)

// configuration from main exe
EXPORT_VAR(GameConfigArray, gs_gameConfigArray)
EXPORT_VAR(BYTE, gs_bombButton)
EXPORT_VAR(BYTE, gs_extraButton)
EXPORT_VAR(DWORD, gs_toggleOsCursorButton)
EXPORT_VAR(DWORD, gs_toggleImGuiButton)
EXPORT_VAR(WCHAR, gs_textureFilePath, [MAX_PATH])
EXPORT_VAR(DWORD, gs_textureBaseHeight)

EXPORT_VAR(DWORD, gs_d3d11_ResizeBuffers_RVA)
EXPORT_VAR(DWORD, gs_d3d11_Present_RVA)

EXPORT_VAR(DWORD, gs_d3d9_CreateDevice_RVA)
EXPORT_VAR(DWORD, gs_d3d9_Reset_RVA)
EXPORT_VAR(DWORD, gs_d3d9_Present_RVA)

EXPORT_VAR(DWORD, gs_d3d8_CreateDevice_RVA)
EXPORT_VAR(DWORD, gs_d3d8_Reset_RVA)
EXPORT_VAR(DWORD, gs_d3d8_Present_RVA)

EXPORT_VAR(DWORD, gs_dinput8_GetDeviceState_RVA)