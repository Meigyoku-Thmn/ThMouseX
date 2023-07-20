#include "macro.h"
#include "framework.h"

#include "Variables.h"

WCHAR   g_currentModuleDirPath[MAX_PATH];

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
bool            g_showImGui;

// configuration from main exe
#pragma data_seg(".SHRCONF")
GameConfigs gs_gameConfigs{};
BYTE        gs_bombButton = 0x58; // VK_X
BYTE        gs_extraButton = 0x43; // VK_C
DWORD       gs_toggleOsCursorButton = 0x4D; // VK_M
DWORD       gs_toggleImGuiButton = 0xC0; // VK_BACK_QUOTE
WCHAR       gs_textureFilePath[MAX_PATH]{};
DWORD       gs_textureBaseHeight = 480;
WCHAR       gs_imGuiFontPath[MAX_PATH]{};
DWORD       gs_imGuiBaseFontSize = 20;
DWORD       gs_imGuiBaseVerticalResolution = 960;
#pragma data_seg()
// make the above segment shared across processes
#pragma comment(linker, "/SECTION:.SHRCONF,RWS")