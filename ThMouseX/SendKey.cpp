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
    SendMessageW(g_hFocusWindow, WM_KEYDOWN, vkCode, MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC));
}

void SendKeyUp(BYTE vkCode) {
    SendMessageW(g_hFocusWindow, WM_KEYUP, vkCode, MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC));
}

void HandleKeyPress(DWORD gameInput, bool& wasPressing, BYTE vkCode) {
    if (gameInput) {
        if (!wasPressing) {
            SendKeyDown(vkCode);
            wasPressing = true;
        }
    } else {
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
        if ((g_currentConfig.InputMethods & InputMethod::SendKey) == InputMethod::None)
            return;
        callbackstore::RegisterPostRenderCallback(TestInputAndSendKeys);
        callbackstore::RegisterUninitializeCallback(CleanUp);
    }
}