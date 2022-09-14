module;

#include "framework.h"

export module core.setcursorhook;

import core.apihijack;

export typedef HCURSOR(WINAPI *SetCursor_t)(HCURSOR hCursor);
export typedef int (WINAPI *ShowCursor_t)(BOOL bShow);

HCURSOR WINAPI MySetCursor(HCURSOR hCursor);
int WINAPI MyShowCursor(BOOL bShow);

export SDLLHook SetCursorHook = {
    .Name = "User32.DLL",
    .UseDefault = false,
    .DefaultFn = NULL,
    .Functions = {
        {.Name = "SetCursor", .HookFn = (DWORD*)MySetCursor},
        {.Name = "ShowCursor", .HookFn = (DWORD*)MyShowCursor},
        {},
    },
};

HCURSOR _hCursor = NULL;
HCURSOR WINAPI MySetCursor(HCURSOR hCursor) {
    auto oldFunc = (SetCursor_t)SetCursorHook.Functions[0].OrigFn;
    if (_hCursor == NULL)
        _hCursor = LoadCursor(NULL, IDC_ARROW);
    return _hCursor;
}

int showCursorCount = 0;
int rs = 0;
int WINAPI MyShowCursor(BOOL bShow) {
    if (showCursorCount == 20) {
        ::rs = bShow == TRUE ? rs + 1 : rs - 1;
        return ::rs;
    }
    showCursorCount++;
    auto oldFunc = (ShowCursor_t)SetCursorHook.Functions[1].OrigFn;
    auto rs = oldFunc(bShow);
    ::rs = rs;
    return rs;
}