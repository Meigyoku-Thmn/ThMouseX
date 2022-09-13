module;

#include "framework.h"

export module core.directx8hook;

import core.apihijack;
import dx8.hook;
import common.var;

// Function pointer types.
typedef DWORD* (WINAPI *Direct3DCreate8_t)(UINT sdk_version);

DWORD* WINAPI MyDirect3DCreate8(UINT sdk_version);

export SDLLHook D3D8Hook = {
    .Name = "d3d8.DLL",
    .UseDefault = false,
    .DefaultFn = NULL,
    .Functions = {
        {.Name = "Direct3DCreate8", .HookFn = (DWORD*)MyDirect3DCreate8},
        {},
    },
};

typedef DWORD* (WINAPI *CreateDirect3D8Detour_t)(DWORD* d3d);

DWORD* WINAPI MyDirect3DCreate8(UINT sdk_version) {
    auto old_func = (Direct3DCreate8_t)D3D8Hook.Functions[0].OrigFn;
    auto d3d = old_func(sdk_version);
    if (g_hModule == NULL)
        return 0;
    g_dxVersion = 8;
    return d3d ? CreateDirect3D8Detour(d3d) : 0;
}