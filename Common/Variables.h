#pragma once
#include "macro.h"
#include "framework.h"
#include "DataTypes.h"

extern WCHAR    g_currentModuleDirPath[MAX_PATH];
extern WCHAR    g_systemDirPath[MAX_PATH];
extern WCHAR    g_currentProcessDirPath[MAX_PATH];
extern WCHAR    g_currentProcessName[MAX_PATH];

// single game config
extern GameConfig  g_currentConfig;

// global game state
extern bool         g_hookApplied;
extern HMODULE      g_targetModule;
extern HMODULE      g_coreModule;
extern HWND         g_hFocusWindow;
extern bool         g_isMinimized;

extern bool         g_leftClicked;
extern bool         g_middleClicked;
extern bool         g_rightClicked;
extern bool         g_forwardClicked;
extern bool         g_backwardClicked;
extern bool         g_scrolledUp;
extern bool         g_scrolledDown;
extern bool         g_scrolledLeft;
extern bool         g_scrolledRight;

extern bool         g_inputEnabled;
extern float        g_pixelRate;
extern FloatPoint   g_pixelOffset;
extern bool         g_showImGui;
// for debugging purpose, not single source of truth
extern POINT        g_playerPos;
extern DoublePoint  g_playerPosRaw;
extern GameInput    g_gameInput;

#ifndef UTILITY_PROJECT
// configuration from main exe
extern GameConfigs  gs_gameConfigs;

extern BYTE     gs_vkCodeForLeftClick;
extern BYTE     gs_vkCodeForMiddleClick;
extern BYTE     gs_vkCodeForRightClick;
extern BYTE     gs_vkCodeForForwardClick;
extern BYTE     gs_vkCodeForBackwardClick;
extern BYTE     gs_vkCodeForScrollUp;
extern BYTE     gs_vkCodeForScrollDown;
extern BYTE     gs_vkCodeForScrollLeft;
extern BYTE     gs_vkCodeForScrollRight;

extern BYTE     gs_toggleMouseControl;
extern BYTE     gs_toggleOsCursorButton;
extern BYTE     gs_toggleImGuiButton;

extern WCHAR    gs_textureFilePath[MAX_PATH];
extern DWORD    gs_textureBaseHeight;
extern WCHAR    gs_imGuiFontPath[MAX_PATH];
extern DWORD    gs_imGuiBaseFontSize;
extern DWORD    gs_imGuiBaseVerticalResolution;
#endif