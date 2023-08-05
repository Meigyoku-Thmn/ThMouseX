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
            auto gameInput = DetermineGameInput();
            if ((gameInput & GameInput::USE_BOMB) == GameInput::USE_BOMB)
                lpKeyState[gs_bombButton] |= 0x80;
            if ((gameInput & GameInput::USE_SPECIAL) == GameInput::USE_SPECIAL)
                lpKeyState[gs_extraButton] |= 0x80;
            if ((gameInput & GameInput::MOVE_LEFT) == GameInput::MOVE_LEFT)
                lpKeyState[VK_LEFT] |= 0x80;
            if ((gameInput & GameInput::MOVE_RIGHT) == GameInput::MOVE_RIGHT)
                lpKeyState[VK_RIGHT] |= 0x80;
            if ((gameInput & GameInput::MOVE_UP) == GameInput::MOVE_UP)
                lpKeyState[VK_UP] |= 0x80;
            if ((gameInput & GameInput::MOVE_DOWN) == GameInput::MOVE_DOWN)
                lpKeyState[VK_DOWN] |= 0x80;
        }
        return rs;
    }
}
