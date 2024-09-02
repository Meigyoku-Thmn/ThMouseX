#include "macro.h"
#include <Windows.h>

#include "Variables.h"

WCHAR   g_currentModuleDirPath[MAX_PATH];
WCHAR   g_systemDirPath[MAX_PATH];
WCHAR   g_currentProcessDirPath[MAX_PATH];
WCHAR   g_currentProcessName[MAX_PATH];

// single game config
GameConfigLocal g_currentConfig;

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
// for debugging purpose, not single source of truth
POINT       g_playerPos;
DoublePoint g_playerPosRaw;
GameInput   g_gameInput;

#ifndef UTILITY_PROJECT
// configuration from main exe
#pragma data_seg(".SHRCONF")
BYTE    gs_vkCodeForLeftClick = 0x58; // VK_X
BYTE    gs_vkCodeForMiddleClick = 0x43; // VK_C
BYTE    gs_vkCodeForRightClick = 0;
BYTE    gs_vkCodeForXButton1Click = 0;
BYTE    gs_vkCodeForXButton2Click = 0;
BYTE    gs_vkCodeForScrollUp = 0;
BYTE    gs_vkCodeForScrollDown = 0;
BYTE    gs_vkCodeForScrollLeft = 0;
BYTE    gs_vkCodeForScrollRight = 0;

BYTE    gs_toggleMouseControl = VK_RBUTTON;
BYTE    gs_toggleOsCursorButton = 0x4D; // VK_M
BYTE    gs_toggleImGuiButton = 0xC0; // VK_BACK_QUOTE

WCHAR   gs_textureFilePath[MAX_PATH]{};
DWORD   gs_textureBaseHeight = 480;
WCHAR   gs_imGuiFontPath[MAX_PATH]{};
DWORD   gs_imGuiBaseFontSize = 20;
DWORD   gs_imGuiBaseVerticalResolution = 960;
#pragma data_seg()
// make the above segment shared across processes
#pragma comment(linker, "/SECTION:.SHRCONF,RWS")
#endif