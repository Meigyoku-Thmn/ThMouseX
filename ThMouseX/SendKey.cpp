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
};
LastState lastState;

// from Autoit source code: https://github.com/ellysh/au3src/blob/35517393091e7d97052d20ccdee8d9d6db36276f/src/sendkeys.cpp#L790
static bool IsVKExtended(UINT key) {
    if (key == VK_INSERT || key == VK_DELETE || key == VK_END || key == VK_DOWN ||
        key == VK_NEXT || key == VK_LEFT || key == VK_RIGHT || key == VK_HOME || key == VK_UP ||
        key == VK_PRIOR || key == VK_DIVIDE || key == VK_APPS || key == VK_LWIN || key == VK_RWIN ||
        key == VK_RMENU || key == VK_RCONTROL || key == VK_SLEEP || key == VK_BROWSER_BACK ||
        key == VK_BROWSER_FORWARD || key == VK_BROWSER_REFRESH || key == VK_BROWSER_STOP ||
        key == VK_BROWSER_SEARCH || key == VK_BROWSER_FAVORITES || key == VK_BROWSER_HOME ||
        key == VK_VOLUME_MUTE || key == VK_VOLUME_DOWN || key == VK_VOLUME_UP || key == VK_MEDIA_NEXT_TRACK ||
        key == VK_MEDIA_PREV_TRACK || key == VK_MEDIA_STOP || key == VK_MEDIA_PLAY_PAUSE ||
        key == VK_LAUNCH_MAIL || key == VK_LAUNCH_MEDIA_SELECT || key == VK_LAUNCH_APP1 || key == VK_LAUNCH_APP2) {
        return true;
    }
    else
        return false;
}

static void SendKeyDown(BYTE vkCode) {
    if ((g_currentConfig.InputMethods & InputMethod::SendMsg) == InputMethod::SendMsg) {
        auto lParam = (MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC) << 16) | 0x00000001;
        if (IsVKExtended(vkCode))
            lParam |= 0x01000000;
        SendMessageW(g_hFocusWindow, WM_KEYDOWN, vkCode, lParam);
    }
    else if ((g_currentConfig.InputMethods & InputMethod::SendInput) == InputMethod::SendInput) {
        if (!g_hFocusWindow || GetForegroundWindow() != g_hFocusWindow)
            return;
        INPUT input{
            .type = INPUT_KEYBOARD,
            .ki = {
                .wVk = vkCode,
                .wScan = (WORD)MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC),
                .dwFlags = DWORD(IsVKExtended(vkCode) ? KEYEVENTF_EXTENDEDKEY : 0),/*
          */},
        };
        SendInput(1, &input, sizeof(INPUT));
    }
}

static void SendKeyUp(BYTE vkCode) {
    if ((g_currentConfig.InputMethods & InputMethod::SendMsg) == InputMethod::SendMsg) {
        auto lParam = (MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC) << 16) | 0xC0000001;
        if (IsVKExtended(vkCode))
            lParam |= 0x01000000;
        SendMessageW(g_hFocusWindow, WM_KEYUP, vkCode, lParam);
    }
    else if ((g_currentConfig.InputMethods & InputMethod::SendInput) == InputMethod::SendInput) {
        if (!g_hFocusWindow || GetForegroundWindow() != g_hFocusWindow)
            return;
        INPUT input{
            .type = INPUT_KEYBOARD,
            .ki = {
                .wVk = vkCode,
                .wScan = (WORD)MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC),
                .dwFlags = DWORD(KEYEVENTF_KEYUP | (IsVKExtended(vkCode) ? KEYEVENTF_EXTENDEDKEY : 0)),/*
          */},
        };
        SendInput(1, &input, sizeof(INPUT));
    }
}

static void HandleKeyPress(GameInput gameInput, bool& wasPressing, BYTE vkCode) {
    if (gameInput != GameInput::NONE) {
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
    auto gameInput = DetermineGameInput();
    HandleKeyPress(gameInput & USE_BOMB, _ref lastState.bomb, gs_bombButton);
    HandleKeyPress(gameInput & USE_SPECIAL, _ref lastState.extra, gs_extraButton);
    HandleKeyPress(gameInput & MOVE_LEFT, _ref lastState.left, VK_LEFT);
    HandleKeyPress(gameInput & MOVE_RIGHT, _ref lastState.right, VK_RIGHT);
    HandleKeyPress(gameInput & MOVE_UP, _ref lastState.up, VK_UP);
    HandleKeyPress(gameInput & MOVE_DOWN, _ref lastState.down, VK_DOWN);
}

static void CleanUp(bool isProcessTerminating) {
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
        using enum InputMethod;
        if ((g_currentConfig.InputMethods & (SendInput | SendMsg)) == None)
            return;
        callbackstore::RegisterPostRenderCallback(TestInputAndSendKeys);
        callbackstore::RegisterUninitializeCallback(CleanUp);
    }
}