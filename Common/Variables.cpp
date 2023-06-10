#include "macro.h"
#include "framework.h"

#include "Variables.h"

HMODULE g_commonModule;
WCHAR   g_currentModuleDirPath[MAX_PATH + 1];

// single game config
GameConfig      g_currentConfig;

// global game state
bool            g_hookApplied;
HMODULE         g_targetModule;
HMODULE         g_coreModule;
HWND            g_hFocusWindow;
bool            g_leftMousePressed;
bool            g_midMousePressed;
bool            g_inputEnabled;
float           g_pixelRate = 1;
FloatPoint      g_pixelOffset{1, 1};

// configuration from main exe
#pragma data_seg(".SHRCONF")
GameConfigArray     gs_gameConfigArray{};
BYTE                gs_bombButton = 0x58; // VK_X
BYTE                gs_extraButton = 0x43; // VK_C
DWORD               gs_toggleOsCursorButton = 0x4D; // VK_M
WCHAR               gs_textureFilePath[MAX_PATH]{};
DWORD               gs_textureBaseHeight = 480;

DWORD               gs_d3d9_CreateDevice_RVA{};
DWORD               gs_d3d9_Reset_RVA{};
DWORD               gs_d3d9_Present_RVA{};

DWORD               gs_d3d8_CreateDevice_RVA{};
DWORD               gs_d3d8_Reset_RVA{};
DWORD               gs_d3d8_Present_RVA{};

DWORD               gs_dinput8_GetDeviceState_RVA{};
#pragma data_seg()
// make the above segment shared across processes
#pragma comment(linker, "/SECTION:.SHRCONF,RWS")