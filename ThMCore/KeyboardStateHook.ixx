module;

#include "framework.h"
#include <mmsystem.h>
#include <vector>

export module core.keyboardstatehook;

import common.minhook;
import core.inputdetermine;
import common.var;
import common.datatype;

namespace minhook = common::minhook;

using namespace std;
using namespace core::inputdetermine;

namespace core::keyboardstatehook {
    BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState);
    decltype(&_GetKeyboardState) OriGetKeyboardState;

    export vector<minhook::HookApiConfig> HookConfig() {
        if ((g_currentConfig.InputMethods & InputMethod::GetKeyboardState) == InputMethod::None)
            return {};
        return {
            {L"USER32.DLL", "GetKeyboardState", &_GetKeyboardState, (PVOID*)&OriGetKeyboardState},
        };
    };

    BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState) {
        auto rs = OriGetKeyboardState(lpKeyState);
        if (rs != FALSE) {
            auto gameInput = DetermineGameInput();
            if (gameInput & USE_BOMB)
                lpKeyState[gs_bombButton] |= 0x80;
            if (gameInput & USE_SPECIAL)
                lpKeyState[gs_extraButton] |= 0x80;
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
}
