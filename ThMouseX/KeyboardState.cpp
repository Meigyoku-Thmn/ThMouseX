#include "framework.h"
#include <vector>

#include "../Common/MinHook.h"
#include "../Common/Variables.h"
#include "../Common/DataTypes.h"
#include "KeyboardState.h"
#include "InputDetermine.h"

namespace minhook = common::minhook;
namespace inputdetermine = core::inputdetermine;

using namespace std;

struct InputRuleItem {
    PBYTE       vkCodePtr;
    BYTE        vkCodeStatic;
    GameInput   input;
};

namespace core::keyboardstate {
    using enum GameInput;

    static BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState);
    static decltype(&_GetKeyboardState) OriGetKeyboardState;

    static InputRuleItem inputRule[]{
        { &gs_vkCodeForLeftClick,       0,          CLICK_LEFT      },
        { &gs_vkCodeForMiddleClick,     0,          CLICK_MIDDLE    },
        { &gs_vkCodeForRightClick,      0,          CLICK_RIGHT     },
        { &gs_vkCodeForXButton1Click,   0,          CLICK_XBUTTON1  },
        { &gs_vkCodeForXButton2Click,   0,          CLICK_XBUTTON2  },
        { &gs_vkCodeForScrollUp,        0,          SCROLL_UP       },
        { &gs_vkCodeForScrollDown,      0,          SCROLL_DOWN     },
        { &gs_vkCodeForScrollLeft,      0,          SCROLL_LEFT     },
        { &gs_vkCodeForScrollRight,     0,          SCROLL_RIGHT    },
        { nil,                          VK_LEFT,    MOVE_LEFT       },
        { nil,                          VK_RIGHT,   MOVE_RIGHT      },
        { nil,                          VK_UP,      MOVE_UP         },
        { nil,                          VK_DOWN,    MOVE_DOWN       },
    };

    void Initialize() {
        using enum InputMethod;
        if ((g_currentConfig.InputMethods & GetKeyboardState) == None)
            return;
        minhook::CreateApiHook(vector<minhook::HookApiConfig>{
            { L"USER32.DLL", "GetKeyboardState", &_GetKeyboardState, &OriGetKeyboardState },
        });
    }

    static BOOL WINAPI _GetKeyboardState(PBYTE lpKeyState) {
        auto rs = OriGetKeyboardState(lpKeyState);
        if (rs != FALSE) {
            using enum GameInput;
            auto gameInput = inputdetermine::DetermineGameInput();
            for (auto const& ruleItem : inputRule) {
                auto vkCode = ruleItem.vkCodePtr == nil ? ruleItem.vkCodeStatic : *ruleItem.vkCodePtr;
                if ((gameInput & ruleItem.input) == ruleItem.input)
                    lpKeyState[vkCode] |= 0x80;
            }
        }
        return rs;
    }
}
