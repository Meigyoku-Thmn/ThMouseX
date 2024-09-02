#include <Windows.h>
#include <vector>
#include <imgui.h>
#include "imgui_impl_win32.h"

#include "../Common/macro.h"
#include "../Common/MinHook.h"
#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/NeoLua.h"
#include "../Common/CallbackStore.h"
#include "../Common/Log.h"
#include "Initialization.h"
#include "MessageQueue.h"

namespace minhook = common::minhook;
namespace neolua = common::neolua;
namespace helper = common::helper;
namespace callbackstore = common::callbackstore;
namespace note = common::log;

using namespace std;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct InputRuleItemVk2SideEffect;
struct InputRuleItemMouseBtn2Function;

using SideEffect = void (*)(bool isUp, bool wantCaptureMouse);
using PostSideEffect = void (*)(InputRuleItemMouseBtn2Function& ruleItem);

enum class MatchStatus {
    None, Up, Down, Trigger
};

struct InputRuleItemVk2SideEffect {
    PBYTE vkCodePtr;
    BYTE vkCodeStatic;
    SideEffect sideEffect;
    bool isOn;
};

struct InputRuleItemMouseBtn2Function {
    BYTE vkCode;
    bool* clickStatePtr;
    bool isOn;
    PostSideEffect nextFrameSideEffect;
};

namespace core::messagequeue {
    HCURSOR WINAPI _SetCursor(HCURSOR hCursor);
    decltype(&_SetCursor) OriSetCursor;
    int WINAPI _ShowCursor(BOOL bShow);
    decltype(&_ShowCursor) OriShowCursor;

    bool isCursorShow = true;
    auto hCursor = LoadCursorA(nil, IDC_ARROW);

    HCURSOR WINAPI _SetCursor(HCURSOR cursor) {
        if (g_showImGui && ImGui_ImplWin32_MouseCursorIsBeingUpdated())
            return OriSetCursor(cursor);
        return nil;
    }

    int WINAPI _ShowCursor(BOOL bShow) {
        return bShow == TRUE ? 0 : -1;
    }

    bool cursorNormalized;
    int cursorVisibility;
    static void ShowCursorEx(bool show) {
        if (show && cursorVisibility < 0)
            cursorVisibility = OriShowCursor(TRUE);
        else if (!show && cursorVisibility >= 0)
            cursorVisibility = OriShowCursor(FALSE);
    }

    static void HideMousePointer() {
        OriSetCursor(nil);
        ShowCursorEx(false);
        isCursorShow = false;
    }

    static void ShowMousePointer() {
        OriSetCursor(hCursor);
        ShowCursorEx(true);
        isCursorShow = true;
    }

    static void NormalizeCursor() {
        // Set cursor visibility to -1, then reset cursor to a normal arrow,
        // to ensure that there is a visible mouse cursor on the game's config dialog
        while (OriShowCursor(TRUE) < 0);
        while (OriShowCursor(FALSE) >= 0);
        cursorVisibility = -1;
        ShowMousePointer();
    }

    static InputRuleItemVk2SideEffect InputRuleVk2SideEffect[]{
        { &gs_toggleImGuiButton, 0, [](bool isUp, UNUSED bool __) {
            if (isUp) return;
            g_showImGui = !g_showImGui;
            if (g_showImGui) {
                g_inputEnabled = false;
                ShowMousePointer();
            }
            else
                HideMousePointer();
        } },
        { &gs_toggleOsCursorButton, 0, [](bool isUp, UNUSED bool __) {
            if (isUp) return;
            if (!g_showImGui)
                isCursorShow ? HideMousePointer() : ShowMousePointer();
        } },
        { &gs_toggleMouseControl, 0, [](bool isUp, bool wantCaptureMouse) {
            if (!isUp) return;
            g_inputEnabled = wantCaptureMouse ? false : !g_inputEnabled;
        } },
    };

    static InputRuleItemMouseBtn2Function InputRuleMouseBtn2ClickState[]{
        { VK_LBUTTON,           &g_leftClicked      },
        { VK_MBUTTON,           &g_middleClicked    },
        { VK_RBUTTON,           &g_rightClicked     },
        { VK_XBUTTON1,          &g_xButton1Clicked  },
        { VK_XBUTTON2,          &g_xButton2Clicked  },
        { SCROLL_UP_EVENT,      &g_scrolledUp       },
        { SCROLL_DOWN_EVENT,    &g_scrolledDown     },
        { SCROLL_LEFT_EVENT,    &g_scrolledLeft     },
        { SCROLL_RIGHT_EVENT,   &g_scrolledRight    },
    };

    static MatchStatus TestVkCode(PMSG e, BYTE vkCode) {
        using enum MatchStatus;
        auto [messageUp, wParamUp, lParamUp, messageDown, wParamDown, lParamDown] = helper::ConvertVkCodeToMessage(vkCode);
        if (messageUp == WM_NULL &&
            messageDown == e->message &&
            (!wParamDown || wParamDown(e->wParam, vkCode)) &&
            (!lParamDown || lParamDown(e->lParam, vkCode))) {
            return Trigger;
        }
        auto matched = true;
        if (messageUp != e->message
            || wParamUp && !wParamUp(e->wParam, vkCode)
            || lParamUp && !lParamUp(e->lParam, vkCode))
            matched = false;
        if (matched)
            return Up;
        matched = true;
        if (messageDown != e->message
            || wParamDown && !wParamDown(e->wParam, vkCode)
            || lParamDown && !lParamDown(e->lParam, vkCode))
            matched = false;
        if (matched)
            return Down;
        return None;
    }

    static bool IsMouseAndKeyboardInput(PMSG e) {
        auto msg = e->message;
        return msg == WM_KEYUP || msg == WM_KEYDOWN ||
            msg == WM_LBUTTONUP || msg == WM_LBUTTONDOWN ||
            msg == WM_MBUTTONUP || msg == WM_MBUTTONDOWN ||
            msg == WM_RBUTTONUP || msg == WM_RBUTTONDOWN ||
            msg == WM_XBUTTONUP || msg == WM_XBUTTONDOWN ||
            msg == WM_MOUSEWHEEL || msg == WM_MOUSEHWHEEL;
    }

    static LRESULT CALLBACK GetMsgProcW(int code, WPARAM wParam, LPARAM lParam) {
        auto e = PMSG(lParam);
        if (code == HC_ACTION && g_hookApplied && g_hFocusWindow && e->hwnd == g_hFocusWindow) {
            using enum MatchStatus;
            if (g_showImGui) {
                ImGui_ImplWin32_WndProcHandler(e->hwnd, e->message, e->wParam, e->lParam);
            }
            if (IsMouseAndKeyboardInput(e)) {
                for (auto& ruleItem : InputRuleVk2SideEffect) {
                    auto vkCode = ruleItem.vkCodePtr == nil ? ruleItem.vkCodeStatic : *ruleItem.vkCodePtr;
                    auto matchStatus = TestVkCode(e, vkCode);
                    if (matchStatus == Trigger) {
                        ruleItem.sideEffect(false, g_showImGui && ImGui::GetIO().WantCaptureMouse);
                    }
                    else if (matchStatus == Up && ruleItem.isOn == true) {
                        ruleItem.isOn = false;
                        ruleItem.sideEffect(true, g_showImGui && ImGui::GetIO().WantCaptureMouse);
                    }
                    else if (matchStatus == Down && ruleItem.isOn == false) {
                        ruleItem.isOn = true;
                        ruleItem.sideEffect(false, g_showImGui && ImGui::GetIO().WantCaptureMouse);
                    }
                    if (matchStatus != None)
                        break;
                }
                for (auto& ruleItem : InputRuleMouseBtn2ClickState) {
                    auto matchStatus = TestVkCode(e, ruleItem.vkCode);
                    if (matchStatus == Trigger) {
                        ruleItem.nextFrameSideEffect = [](auto& _ruleItem) { *_ruleItem.clickStatePtr = false; };
                        *ruleItem.clickStatePtr = true;
                    }
                    else if (matchStatus == Up && ruleItem.isOn == true) {
                        ruleItem.isOn = false;
                        *ruleItem.clickStatePtr = false;
                    }
                    else if (matchStatus == Down && ruleItem.isOn == false) {
                        ruleItem.isOn = true;
                        *ruleItem.clickStatePtr = true;
                    }
                    if (matchStatus != None)
                        break;
                }
            }
        }
        return CallNextHookEx(nil, code, wParam, lParam);
    }

    static LRESULT CALLBACK CallWndRetProcW(int code, WPARAM wParam, LPARAM lParam) {
        if (static auto initialized = false; !initialized) {
            initialized = true;
            core::Initialize();
        }
        if (code == HC_ACTION && g_hookApplied) {
            if (!cursorNormalized) {
                cursorNormalized = true;
                NormalizeCursor();
            }
            auto e = (PCWPRETSTRUCT)lParam;
            if (g_showImGui) {
                ImGui_ImplWin32_WndProcHandler(e->hwnd, e->message, e->wParam, e->lParam);
            }
            else if (e->message == WM_SETCURSOR && !g_showImGui) {
                if (LOWORD(e->lParam) == HTCLIENT) {
                    if (isCursorShow)
                        ShowMousePointer();
                    else
                        HideMousePointer();
                }
                else {
                    ShowCursorEx(true);
                    DefWindowProcW(e->hwnd, e->message, e->wParam, e->lParam);
                }
            }
            else if (e->message == WM_SIZE && g_hFocusWindow) {
                if (e->wParam == SIZE_MINIMIZED) {
                    g_isMinimized = true;
                }
                else if (e->wParam == SIZE_RESTORED && g_isMinimized) {
                    g_isMinimized = false;
                    callbackstore::TriggerClearMeasurementFlagsCallbacks();
                }
            }
        }
        return CallNextHookEx(nil, code, wParam, lParam);
    }

    static bool CheckHookProcHandle(HHOOK handle) {
        if (handle != nil)
            return true;
        helper::ReportLastError(APP_NAME ": SetWindowsHookEx Error");
        return false;
    }

    HHOOK GetMsgProcHandle;
    HHOOK CallWndRetProcHandle;

    bool InstallHooks() {
        GetMsgProcHandle = SetWindowsHookExW(WH_GETMESSAGE, GetMsgProcW, g_coreModule, NULL);
        if (!CheckHookProcHandle(GetMsgProcHandle))
            return false;
        CallWndRetProcHandle = SetWindowsHookExW(WH_CALLWNDPROCRET, CallWndRetProcW, g_coreModule, NULL);
        if (!CheckHookProcHandle(CallWndRetProcHandle))
            return false;
        return true;
    }

    void RemoveHooks() {
        // unregister hooks.
        UnhookWindowsHookEx(GetMsgProcHandle);
        UnhookWindowsHookEx(CallWndRetProcHandle);
        // force all top-level windows to process a message, therefore force all processes to unload the DLL.
        DWORD _;
        SendMessageTimeoutW(HWND_BROADCAST, WM_NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &_);
    }

    static void PostRenderCallback() {
        for (auto& ruleItem : InputRuleMouseBtn2ClickState) {
            if (ruleItem.nextFrameSideEffect) {
                ruleItem.nextFrameSideEffect(ruleItem);
                ruleItem.nextFrameSideEffect = nil;
            }
        }
        static bool callbackDone = false;
        if (cursorNormalized && !callbackDone) {
            callbackDone = true;
            HideMousePointer();
        }
    }

    void Initialize() {
        // Hide the mouse cursor when D3D is running, but only after cursor normalization
        callbackstore::RegisterPostRenderCallback(PostRenderCallback);
        minhook::CreateApiHook(vector<minhook::HookApiConfig>{
            { L"USER32.DLL", "SetCursor", & _SetCursor, & OriSetCursor },
            { L"USER32.DLL", "ShowCursor", &_ShowCursor, &OriShowCursor },
        });
    }
}