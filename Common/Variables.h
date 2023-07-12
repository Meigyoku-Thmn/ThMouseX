#pragma once
#include "macro.h"
#include "framework.h"
#include "DataTypes.h"

extern WCHAR    g_currentModuleDirPath[MAX_PATH];

// single game config
extern GameConfig  g_currentConfig;

// global game state
extern bool        g_hookApplied;
extern HMODULE     g_targetModule;
extern HMODULE     g_coreModule;
extern HWND        g_hFocusWindow;
extern bool        g_leftMousePressed;
extern bool        g_midMousePressed;
extern bool        g_inputEnabled;
extern float       g_pixelRate;
extern FloatPoint  g_pixelOffset;
extern bool        g_showImGui;

// configuration from main exe
extern GameConfigs  gs_gameConfigs;
extern BYTE         gs_bombButton;
extern BYTE         gs_extraButton;
extern DWORD        gs_toggleOsCursorButton;
extern DWORD        gs_toggleImGuiButton;
extern WCHAR        gs_textureFilePath[MAX_PATH];
extern DWORD        gs_textureBaseHeight;