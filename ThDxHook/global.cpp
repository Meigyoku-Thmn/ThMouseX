#include "stdafx.h"
#include "global.h"

HWND g_hFocusWindow = 0;
BOOL g_mouseDown = 0;
BOOL g_midMouseDown = 0;
GameConfig g_currentGameConfig = {"", {0, {0, 0, 0, 0, 0, 0, 0, 0}}, Int_DataType, 1.0f, {0.0f, 0.0f}, FALSE};
DWORD g_boomButton = 0;
DWORD g_extraButton = 0;
BOOL g_working = 0;
BOOL keyBoard = FALSE;
BOOL windowed = TRUE;
FLOAT g_currentScale = 1.0f;

int dxVersion = 9;
bool offsetIsRelative = false;
DWORD baseOfCode = 0;
DWORD firstOffsetDirection = 1;


#pragma data_seg(".CONFIGDATA") //Shared data among all instances.
GameConfigArray gs_gameConfigArray = {0};
DWORD gs_boomButton = 0;
DWORD gs_extraButton = 0;
char gs_textureFilePath[TEXTURE_FILE_PATH_LEN] = {0};
#pragma data_seg()
#pragma comment(linker, "/SECTION:.CONFIGDATA,RWS") //linker directive

