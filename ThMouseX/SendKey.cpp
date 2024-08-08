#include "SendKey.h"
#include "framework.h"

#include <span>

#include "../Common/macro.h"
#include "../Common/Variables.h"
#include "../Common/CallbackStore.h"
#include "../Common/Helper.h"
#include "InputDetermine.h"

namespace callbackstore = common::callbackstore;
namespace helper = common::helper;
namespace inputdetermine = core::inputdetermine;

using namespace std;

struct InputRuleStateItem {
    PBYTE       vkCodePtr;
    BYTE        vkCodeStatic;
    bool        lastState;
    GameInput   input;
};

static InputRuleStateItem inputRule[]{
     { &gs_vkCodeForLeftClick,       0,         false,  GameInput::CLICK_LEFT      },
     { &gs_vkCodeForMiddleClick,     0,         false,  GameInput::CLICK_MIDDLE    },
     { &gs_vkCodeForRightClick,      0,         false,  GameInput::CLICK_RIGHT     },
     { &gs_vkCodeForXButton1Click,   0,         false,  GameInput::CLICK_XBUTTON1  },
     { &gs_vkCodeForXButton2Click,   0,         false,  GameInput::CLICK_XBUTTON2  },
     { &gs_vkCodeForScrollUp,        0,         false,  GameInput::SCROLL_UP       },
     { &gs_vkCodeForScrollDown,      0,         false,  GameInput::SCROLL_DOWN     },
     { &gs_vkCodeForScrollLeft,      0,         false,  GameInput::SCROLL_LEFT     },
     { &gs_vkCodeForScrollRight,     0,         false,  GameInput::SCROLL_RIGHT    },
     { nil,                          VK_LEFT,   false,  GameInput::MOVE_LEFT       },
     { nil,                          VK_RIGHT,  false,  GameInput::MOVE_RIGHT      },
     { nil,                          VK_UP,     false,  GameInput::MOVE_UP         },
     { nil,                          VK_DOWN,   false,  GameInput::MOVE_DOWN       },
};

static void SendKeyDown(BYTE vkCode) {
    using enum InputMethod;
    if ((g_currentConfig.InputMethods & SendMsg) == SendMsg) {
        auto lParam = (MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC_EX) << 16) | 0x00000001;
        if (helper::ShouldBeVkExtended(vkCode))
            lParam |= 0x01000000;
        SendMessageW(g_hFocusWindow, WM_KEYDOWN, helper::NormalizeLeftRightVkCode(vkCode), lParam);
    }
    else if ((g_currentConfig.InputMethods & SendInput) == SendInput) {
        if (!g_hFocusWindow || GetForegroundWindow() != g_hFocusWindow)
            return;
        INPUT input{
            .type = INPUT_KEYBOARD,
            .ki = {
                .wVk = helper::NormalizeLeftRightVkCode(vkCode),
                .wScan = (WORD)MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC_EX),
                .dwFlags = DWORD(helper::ShouldBeVkExtended(vkCode) ? KEYEVENTF_EXTENDEDKEY : 0),/*
          */},
        };
        ::SendInput(1, &input, sizeof(INPUT));
    }
}

static void SendKeyUp(BYTE vkCode) {
    using enum InputMethod;
    if ((g_currentConfig.InputMethods & SendMsg) == SendMsg) {
        auto lParam = (MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC_EX) << 16) | 0xC0000001;
        if (helper::ShouldBeVkExtended(vkCode))
            lParam |= 0x01000000;
        SendMessageW(g_hFocusWindow, WM_KEYUP, helper::NormalizeLeftRightVkCode(vkCode), lParam);
    }
    else if ((g_currentConfig.InputMethods & SendInput) == SendInput) {
        if (!g_hFocusWindow || GetForegroundWindow() != g_hFocusWindow)
            return;
        INPUT input{
            .type = INPUT_KEYBOARD,
            .ki = {
                .wVk = helper::NormalizeLeftRightVkCode(vkCode),
                .wScan = (WORD)MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC_EX),
                .dwFlags = DWORD(KEYEVENTF_KEYUP | (helper::ShouldBeVkExtended(vkCode) ? KEYEVENTF_EXTENDEDKEY : 0)),/*
          */},
        };
        ::SendInput(1, &input, sizeof(INPUT));
    }
}

static void HandleKeyPress(GameInput gameInput, bool& wasPressing, BYTE vkCode) {
    using enum GameInput;
    if (gameInput != NONE) {
        if (!wasPressing) {
            SendKeyDown(vkCode);
            wasPressing = true;
        }
    }
    else {
        if (wasPressing) {
            SendKeyUp(vkCode);
            wasPressing = false;
        }
    }
}

static void TestInputAndSendKeys() {
    using enum GameInput;
    auto gameInput = inputdetermine::DetermineGameInput();
    for (auto& ruleItem : inputRule) {
        auto vkCode = ruleItem.vkCodePtr == nil ? ruleItem.vkCodeStatic : *ruleItem.vkCodePtr;
        HandleKeyPress(gameInput & ruleItem.input, ruleItem.lastState, vkCode);
    }
}

static void CleanUp(bool isProcessTerminating) {
    if (isProcessTerminating)
        return;
    for (const auto& ruleItem : inputRule) {
        auto vkCode = ruleItem.vkCodePtr == nil ? ruleItem.vkCodeStatic : *ruleItem.vkCodePtr;
        SendKeyUp(vkCode);
    }
}

namespace core::sendkey {
    void Initialize() {
        using enum InputMethod;
        if ((g_currentConfig.InputMethods & (SendInput | SendMsg)) == None)
            return;
        callbackstore::RegisterPostRenderCallback(TestInputAndSendKeys);
        callbackstore::RegisterUninitializeCallback(CleanUp);
    }
}