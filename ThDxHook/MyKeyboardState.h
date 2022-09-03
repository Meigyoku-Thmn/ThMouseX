#pragma once
BOOL WINAPI MyGetKeyboardState(_Out_ PBYTE lpKeyState);

typedef BOOL(WINAPI * OriGetKeyboardState)(_Out_ PBYTE lpKeyState);
extern OriGetKeyboardState getKeyboardState;