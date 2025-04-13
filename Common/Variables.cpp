#include "macro.h"
#include <Windows.h>

#include "Variables.h"

WCHAR   g_currentModulePath[MAX_PATH];
WCHAR   g_currentModuleDirPath[MAX_PATH];
WCHAR   g_systemDirPath[MAX_PATH];
WCHAR   g_currentProcessDirPath[MAX_PATH];
WCHAR   g_currentProcessName[MAX_PATH];

// single game config
GameConfig      g_gameConfig;
CommonConfig    g_commonConfig;

// global game state
bool        g_hookApplied;
HMODULE     g_targetModule;
HMODULE     g_coreModule;
HWND        g_hFocusWindow;
bool        g_isMinimized;

bool        g_leftClicked;
bool        g_middleClicked;
bool        g_rightClicked;
bool        g_xButton1Clicked;
bool        g_xButton2Clicked;
bool        g_scrolledUp;
bool        g_scrolledDown;
bool        g_scrolledLeft;
bool        g_scrolledRight;

bool        g_inputEnabled;
float       g_pixelRate = 1;
FloatPoint  g_pixelOffset{ 1, 1 };
bool        g_showImGui;

// should be moved to GameConfig or CommonConfig(?)
MovementAlgorithm   g_movementAlgorithm;

// for debugging purpose, not single source of truth
POINT       g_playerPos;
DoublePoint g_playerPosRaw;
GameInput   g_gameInput;