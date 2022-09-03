#include "stdafx.h"
#include "MyDirect3DCreate8.h"
// #include "MyDirect3D8.h"

#include "../DX8Hook/subGlobal.h"
#include "../DX8Hook/DX8Hook.h"
#include "global.h"

// Function pointer types.
typedef DWORD* (WINAPI *Direct3DCreate8_t)(UINT sdk_version);

SDLLHook D3D8Hook = {
    "d3d8.DLL",
    false, NULL,		// Default hook disabled, NULL function pointer.
    {
        {"Direct3DCreate8", (DWORD*)MyDirect3DCreate8},
        {NULL, NULL}
    }
};

typedef DWORD* (WINAPI *CreateDirect3D8Detour_t)(DWORD* d3d);

// I have to done like this because of name collision between DX8 and DX9 >_<
// If you have better solution, please tell me!
DWORD* WINAPI MyDirect3DCreate8(UINT sdk_version) {
    //WriteToLog( "ThDxHook: MyDirect3DCreate9 called.\n" );

    Direct3DCreate8_t old_func = (Direct3DCreate8_t)D3D8Hook.Functions[0].OrigFn;
    DWORD* d3d = old_func(sdk_version);

    if (_hModule == NULL) return 0;

    dxVersion = 8;
    return d3d ? CreateDirect3D8Detour(d3d) : 0;
}