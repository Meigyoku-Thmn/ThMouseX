#include "SendKey.h"
#include "framework.h"

#include "../Common/macro.h"
#include "../Common/Variables.h"
#include "../Common/CallbackStore.h"
#include "../Common/Helper.h"
#include "InputDetermine.h"

using namespace core::inputdetermine;

namespace callbackstore = common::callbackstore;
namespace helper = common::helper;

struct LastState {
    bool bomb;
    bool extra;
    bool left;
    bool right;
    bool up;
    bool down;
};
LastState lastState;

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
    auto gameInput = DetermineGameInput();
    HandleKeyPress(gameInput & CLICK_LEFT, lastState.bomb, gs_vkCodeForLeftClick);
    HandleKeyPress(gameInput & CLICK_MIDDLE, lastState.extra, gs_vkCodeForMiddleClick);
    HandleKeyPress(gameInput & MOVE_LEFT, lastState.left, VK_LEFT);
    HandleKeyPress(gameInput & MOVE_RIGHT, lastState.right, VK_RIGHT);
    HandleKeyPress(gameInput & MOVE_UP, lastState.up, VK_UP);
    HandleKeyPress(gameInput & MOVE_DOWN, lastState.down, VK_DOWN);
}

static void CleanUp(bool isProcessTerminating) {
    if (isProcessTerminating)
        return;
    SendKeyUp(gs_vkCodeForLeftClick);
    SendKeyUp(gs_vkCodeForMiddleClick);
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