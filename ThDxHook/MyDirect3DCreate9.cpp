#include "stdafx.h"

#include "MyDirect3DCreate9.h"
#include "MyDirect3D9.h"

// Function pointer types.
typedef IDirect3D9* (WINAPI *Direct3DCreate9_t)(UINT sdk_version);

enum {
    D3DFN_Direct3DCreate9 = 0
};

SDLLHook D3DHook = {
    "d3d9.DLL",
    false, NULL,		// Default hook disabled, NULL function pointer.
    {
        {"Direct3DCreate9", (DWORD*)MyDirect3DCreate9},
        {NULL, NULL}
    }
};

IDirect3D9* WINAPI MyDirect3DCreate9(UINT sdk_version) {
    //WriteToLog( "ThDxHook: MyDirect3DCreate9 called.\n" );

    Direct3DCreate9_t old_func = (Direct3DCreate9_t)D3DHook.Functions[D3DFN_Direct3DCreate9].OrigFn;
    IDirect3D9* d3d = old_func(sdk_version);

    return d3d ? new MyDirect3D9(d3d) : 0;
}