#pragma once

#include "apihijack.h"

HRESULT WINAPI MyDirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID *ppvOut,
    LPUNKNOWN punkOuter
);

HRESULT WINAPI MyDirectInputCreateW(
    HINSTANCE hinst,
    DWORD dwVersion,
    LPDIRECTINPUTW* lplpDirectInput,
    LPUNKNOWN punkOuter
);

extern SDLLHook DInput8Hook;
extern SDLLHook DInputHook;