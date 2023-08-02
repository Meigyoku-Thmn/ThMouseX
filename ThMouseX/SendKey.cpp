#include "SendKey.h"
#include "framework.h"

#include "../Common/macro.h"
#include "../Common/Variables.h"
#include "../Common/CallbackStore.h"
#include "InputDetermine.h"

using namespace core::inputdetermine;

namespace callbackstore = common::callbackstore;

struct LastState {
    bool bomb;
    bool extra;
    bool left;
    bool right;
    bool up;
    bool down;
} lastState;

void SendKeyDown(BYTE vkCode) {
    if ((g_currentConfig.InputMethods & InputMethod::SendMsg) == InputMethod::SendMsg) {
        auto lParam = MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC) << 16 | 0x00000001;
        SendMessageW(g_hFocusWindow, WM_KEYDOWN, vkCode, lParam);
    }
    else if ((g_currentConfig.InputMethods & InputMethod::SendKey) == InputMethod::SendKey) {
        INPUT input{
            .type = INPUT_KEYBOARD,
            .ki = {
                .wVk = vkCode,
                .wScan = (WORD)MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC),
                .dwFlags = KEYEVENTF_EXTENDEDKEY,
            },
        };
        SendInput(1, &input, sizeof(INPUT));
    }
}

void SendKeyUp(BYTE vkCode) {
    if ((g_currentConfig.InputMethods & InputMethod::SendMsg) == InputMethod::SendMsg) {
        auto lParam = MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC) << 16 | 0xC0000001;
        SendMessageW(g_hFocusWindow, WM_KEYUP, vkCode, lParam);
    }
    else if ((g_currentConfig.InputMethods & InputMethod::SendKey) == InputMethod::SendKey) {
        INPUT input{
            .type = INPUT_KEYBOARD,
            .ki = {
                .wVk = vkCode,
                .wScan = (WORD)MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC),
                .dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_EXTENDEDKEY,
            },
        };
        SendInput(1, &input, sizeof(INPUT));
    }
}

void HandleKeyPress(DWORD gameInput, bool& wasPressing, BYTE vkCode) {
    if (gameInput) {
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

void TestInputAndSendKeys() {
    auto gameInput = DetermineGameInput();
    HandleKeyPress(gameInput & USE_BOMB, _ref lastState.bomb, gs_bombButton);
    HandleKeyPress(gameInput & USE_SPECIAL, _ref lastState.extra, gs_extraButton);
    HandleKeyPress(gameInput & MOVE_LEFT, _ref lastState.left, VK_LEFT);
    HandleKeyPress(gameInput & MOVE_RIGHT, _ref lastState.right, VK_RIGHT);
    HandleKeyPress(gameInput & MOVE_UP, _ref lastState.up, VK_UP);
    HandleKeyPress(gameInput & MOVE_DOWN, _ref lastState.down, VK_DOWN);
}

void CleanUp(bool isProcessTerminating) {
    if (isProcessTerminating)
        return;
    SendKeyUp(gs_bombButton);
    SendKeyUp(gs_extraButton);
    SendKeyUp(VK_LEFT);
    SendKeyUp(VK_RIGHT);
    SendKeyUp(VK_UP);
    SendKeyUp(VK_DOWN);
}

namespace core::sendkey {
    void Initialize() {
        if ((g_currentConfig.InputMethods & (InputMethod::SendKey | InputMethod::SendMsg)) == InputMethod::None)
            return;
        callbackstore::RegisterPostRenderCallback(TestInputAndSendKeys);
        callbackstore::RegisterUninitializeCallback(CleanUp);
    }
}