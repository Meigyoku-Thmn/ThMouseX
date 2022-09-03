#pragma once

#include "GameConfigArray.h"
#include <Windows.h>

#define TEXTURE_FILE_PATH_LEN 256

struct FPOINT {
    float x;
    float y;
};


extern HWND g_hFocusWindow;
extern BOOL g_mouseDown;
extern BOOL g_midMouseDown;
extern GameConfig g_currentGameConfig;
extern DWORD g_boomButton;
extern DWORD g_extraButton;
extern BOOL g_working;
extern FLOAT g_currentScale;
extern bool offsetIsRelative;
extern DWORD baseOfCode;
extern DWORD firstOffsetDirection;

extern GameConfigArray gs_gameConfigArray;
extern DWORD gs_boomButton;
extern DWORD gs_extraButton;
extern char gs_textureFilePath[TEXTURE_FILE_PATH_LEN];

extern BOOL keyBoard;
extern BOOL windowed;
extern int dxVersion;
