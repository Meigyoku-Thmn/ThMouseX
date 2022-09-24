module;

#include "framework.h"
#include <mmsystem.h>
#include <vector>

export module core.lowlevelinputhook;

import common.minhook;
import core.inputdeterminte;
import common.var;

using namespace std;

constexpr auto VK_X = 0x58;
constexpr auto VK_C = 0x43;

BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState);
decltype(&_GetKeyboardState) OriGetKeyboardState;

export vector<MHookApiConfig> LowLevelInputHookConfig{
    {L"USER32.DLL", "GetKeyboardState", &_GetKeyboardState, (PVOID*)&OriGetKeyboardState},
};

BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState) {
    auto rs = OriGetKeyboardState(lpKeyState);
    if (!g_handledByDirectInput && !g_handledByGetAsyncKeyState && rs != 0) {
        g_handledByGetKeyboardState = true;
        auto gameInput = DetermineGameInput();
        if (gameInput & USE_BOMB)
            lpKeyState[VK_X] |= 0x80;
        if (gameInput & USE_SPECIAL)
            lpKeyState[VK_C] |= 0x80;
        if (gameInput & MOVE_LEFT)
            lpKeyState[VK_LEFT] |= 0x80;
        if (gameInput & MOVE_RIGHT)
            lpKeyState[VK_RIGHT] |= 0x80;
        if (gameInput & MOVE_UP)
            lpKeyState[VK_UP] |= 0x80;
        if (gameInput & MOVE_DOWN)
            lpKeyState[VK_DOWN] |= 0x80;
    }
    return rs;
}
