#include "stdafx.h"

#include "MySetCursor.h"

// Function pointer types.
typedef HCURSOR(WINAPI *SetCursor_t)(HCURSOR hCursor);

typedef int (WINAPI *ShowCursor_t)(BOOL bShow);

SDLLHook SetCursorHook = {
    "User32.DLL",
    false, NULL,		// Default hook disabled, NULL function pointer.
    {
        {"SetCursor", (DWORD*)MySetCursor},
        {"ShowCursor", (DWORD*)MyShowCursor},
        {NULL, NULL}
    }
};
HCURSOR _hCursor = 0;
HCURSOR WINAPI MySetCursor(HCURSOR hCursor) {
    SetCursor_t oldFunc = (SetCursor_t)SetCursorHook.Functions[0].OrigFn;
    if (_hCursor == 0)
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
    ShowCursor_t oldFunc = (ShowCursor_t)SetCursorHook.Functions[1].OrigFn;
    int rs = oldFunc(bShow);
    ::rs = rs;
    return rs;
}