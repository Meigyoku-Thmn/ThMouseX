#pragma once

#include "apihijack.h"

HCURSOR WINAPI MySetCursor(HCURSOR hCursor);

int WINAPI MyShowCursor(BOOL bShow);

extern SDLLHook SetCursorHook;
