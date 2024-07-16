#include "framework.h"
#include <mmsystem.h>
#include <vector>

#include "../Common/MinHook.h"
#include "../Common/Variables.h"
#include "../Common/DataTypes.h"
#include "KeyboardState.h"
#include "InputDetermine.h"

namespace minhook = common::minhook;

using namespace std;
using namespace core::inputdetermine;

namespace core::keyboardstate {
    BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState);
    decltype(&_GetKeyboardState) OriGetKeyboardState;

    void Initialize() {
        if ((g_currentConfig.InputMethods & InputMethod::GetKeyboardState) == InputMethod::None)
            return;
        minhook::CreateApiHook(vector<minhook::HookApiConfig>{
            {L"USER32.DLL", "GetKeyboardState", &_GetKeyboardState, (PVOID*)&OriGetKeyboardState},
        });
    }

    BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState) {
        auto rs = OriGetKeyboardState(lpKeyState);
        if (rs != FALSE) {
            using enum GameInput;
            auto gameInput = DetermineGameInput();
            if ((gameInput & USE_BOMB) == USE_BOMB)
                lpKeyState[gs_bombButton] |= 0x80;
            if ((gameInput & USE_SPECIAL) == USE_SPECIAL)
                lpKeyState[gs_extraButton] |= 0x80;
            if ((gameInput & MOVE_LEFT) == MOVE_LEFT)
                lpKeyState[VK_LEFT] |= 0x80;
            if ((gameInput & MOVE_RIGHT) == MOVE_RIGHT)
                lpKeyState[VK_RIGHT] |= 0x80;
            if ((gameInput & MOVE_UP) == MOVE_UP)
                lpKeyState[VK_UP] |= 0x80;
            if ((gameInput & MOVE_DOWN) == MOVE_DOWN)
                lpKeyState[VK_DOWN] |= 0x80;
        }
        return rs;
    }
}
