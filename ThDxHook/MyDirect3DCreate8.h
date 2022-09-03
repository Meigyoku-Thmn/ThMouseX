#pragma once

#include "apihijack.h"
#include <Windows.h>

DWORD* WINAPI MyDirect3DCreate8(UINT sdk_version);

extern SDLLHook D3D8Hook;
