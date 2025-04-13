#pragma once
#include "macro.h"
#include <Windows.h>
#include "DataTypes.h"

extern WCHAR    g_currentModulePath[MAX_PATH];
extern WCHAR    g_currentModuleDirPath[MAX_PATH];
extern WCHAR    g_systemDirPath[MAX_PATH];
extern WCHAR    g_currentProcessDirPath[MAX_PATH];
extern WCHAR    g_currentProcessName[MAX_PATH];

// single game config
extern GameConfig   g_gameConfig;
extern CommonConfig g_commonConfig;

// global game state
extern bool         g_hookApplied;
extern HMODULE      g_targetModule;
extern HMODULE      g_coreModule;
extern HWND         g_hFocusWindow;
extern bool         g_isMinimized;

extern bool         g_leftClicked;
extern bool         g_middleClicked;
extern bool         g_rightClicked;
extern bool         g_xButton1Clicked;
extern bool         g_xButton2Clicked;
extern bool         g_scrolledUp;
extern bool         g_scrolledDown;
extern bool         g_scrolledLeft;
extern bool         g_scrolledRight;

extern bool         g_inputEnabled;
extern float        g_pixelRate;
extern FloatPoint   g_pixelOffset;
extern bool         g_showImGui;

// should be moved to GameConfig or CommonConfig(?)
extern MovementAlgorithm    g_movementAlgorithm;

// for debugging purpose, not single source of truth
extern POINT        g_playerPos;
extern DoublePoint  g_playerPosRaw;
extern GameInput    g_gameInput;