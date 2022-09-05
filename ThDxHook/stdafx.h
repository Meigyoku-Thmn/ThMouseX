// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <Psapi.h>
#pragma comment( lib, "psapi.lib" )
#include <tlhelp32.h>
//#include <d3d9.h>
#include "Include/d3d9.h"
#pragma comment(lib, "Lib/x86/d3d9.lib")
//#include <D3dx9core.h>
#include "Include/d3dx9core.h"
#pragma comment(lib, "Lib/x86/D3dx9.lib")
#include "Include/dinput.h"
#pragma comment(lib, "Lib/x86/dinput8.lib")
#include <mmsystem.h>

#pragma comment(lib, "Lib/x86/dxguid.lib")

// TODO: reference additional headers your program requires here

#include "CustomType.h"

#define _ref