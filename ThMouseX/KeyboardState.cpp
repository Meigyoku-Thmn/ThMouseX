#include <Windows.h>
#include <vector>

#include "../Common/MinHook.h"
#include "../Common/Variables.h"
#include "../Common/DataTypes.h"
#include "KeyboardState.h"
#include "InputDetermine.h"

namespace minhook = common::minhook;
namespace inputdetermine = core::inputdetermine;

static CommonConfig& g_c = g_commonConfig;

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
        { &g_c.VkCodeForLeftClick,      0,          CLICK_LEFT      },
        { &g_c.VkCodeForMiddleClick,    0,          CLICK_MIDDLE    },
        { &g_c.VkCodeForRightClick,     0,          CLICK_RIGHT     },
        { &g_c.VkCodeForXButton1Click,  0,          CLICK_XBUTTON1  },
        { &g_c.VkCodeForXButton2Click,  0,          CLICK_XBUTTON2  },
        { &g_c.VkCodeForScrollUp,       0,          SCROLL_UP       },
        { &g_c.VkCodeForScrollDown,     0,          SCROLL_DOWN     },
        { &g_c.VkCodeForScrollLeft,     0,          SCROLL_LEFT     },
        { &g_c.VkCodeForScrollRight,    0,          SCROLL_RIGHT    },
        { nil,                          VK_LEFT,    MOVE_LEFT       },
        { nil,                          VK_RIGHT,   MOVE_RIGHT      },
        { nil,                          VK_UP,      MOVE_UP         },
        { nil,                          VK_DOWN,    MOVE_DOWN       },
    };

    void Initialize() {
        if ((g_gameConfig.InputMethods & InputMethod_GetKeyboardState) == InputMethod_None)
            return;
        minhook::CreateApiHook(vector<minhook::HookApiConfig>{
            { L"USER32.DLL", "GetKeyboardState", &_GetKeyboardState, &OriGetKeyboardState, APP_NAME "_GetKeyboardState" },
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
