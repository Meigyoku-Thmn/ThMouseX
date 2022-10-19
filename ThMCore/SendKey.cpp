#include "framework.h"
#include "macro.h"

import dx8.hook;
import core.directx9hook;
import common.var;
import common.datatype;
import core.inputdetermine;
import common.minhook;

using namespace core::inputdetermine;

namespace minhook = common::minhook;
namespace directx8 = dx8::hook;
namespace directx9 = core::directx9hook;

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

void HandleHoldKey(DWORD gameInput, bool& lastState, BYTE vkCode) {
    if (gameInput) {
        if (!lastState) {
            SendKeyDown(vkCode);
            lastState = true;
        }
    } else {
        if (lastState) {
            SendKeyUp(vkCode);
            lastState = false;
        }
    }
}

void HandleTriggerKey(DWORD gameInput, bool& lastState, BYTE vkCode) {
    if (gameInput) {
        SendKeyDown(vkCode);
        lastState = true;
    } else if (lastState) {
        SendKeyUp(vkCode);
        lastState = false;
    }
}

void TestInputAndSendKeys() {
    if ((g_currentConfig.InputMethods & InputMethod::SendKey) == InputMethod::None)
        return;
    auto gameInput = DetermineGameInput();
    HandleTriggerKey(gameInput & USE_BOMB, _ref lastState.bomb, gs_bombButton);
    HandleTriggerKey(gameInput & USE_SPECIAL, _ref lastState.extra, gs_extraButton);
    HandleHoldKey(gameInput & MOVE_LEFT, _ref lastState.left, VK_LEFT);
    HandleHoldKey(gameInput & MOVE_RIGHT, _ref lastState.right, VK_RIGHT);
    HandleHoldKey(gameInput & MOVE_UP, _ref lastState.up, VK_UP);
    HandleHoldKey(gameInput & MOVE_DOWN, _ref lastState.down, VK_DOWN);
}

void CleanUp(bool isProcessTerminating) {
    if ((g_currentConfig.InputMethods & InputMethod::SendKey) == InputMethod::None)
        return;
    if (isProcessTerminating)
        return;
    SendKeyUp(gs_bombButton);
    SendKeyUp(gs_extraButton);
    SendKeyUp(VK_LEFT);
    SendKeyUp(VK_RIGHT);
    SendKeyUp(VK_UP);
    SendKeyUp(VK_DOWN);
}

struct OnInit {
    OnInit() {
        directx8::RegisterPostRenderCallbacks(TestInputAndSendKeys);
        directx9::RegisterPostRenderCallbacks(TestInputAndSendKeys);
        minhook::RegisterUninitializeCallback(CleanUp);
    }
} _;