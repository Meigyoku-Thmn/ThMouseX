#include "framework.h"
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

using Action = function<void()>;
using SideEffect = void (*)(bool isUp, bool wantCaptureMouse);

struct InputRuleItemVk2SideEffect {
    PBYTE vkCodePtr;
    BYTE vkCodeStatic;
    SideEffect sideEffect;
    bool isOn;
};

struct InputRuleItemMouseBtn2Function {
    BYTE vkCode;
    bool* clickState;
    bool isOn;
};

enum class EventUseCase {
    ImGuiBtn, OsCursorBtn, MouseControlBtn,
    MouseLeftClick, MouseMiddleClick, MouseRightClick,
    MouseForwardClick, MouseBackwardClick,
    MouseScrollUp, MouseScrollDown, MouseScrollLeft, MouseScrollRight,
};

template <EventUseCase T>
static void HandleMousePress(PMSG e, DWORD msgDown, DWORD msgUp, const Action& downAction, const Action& upAction) {
    static bool isOn = false;
    if (e->message == msgDown && isOn == false) {
        isOn = true;
        if (downAction) downAction();
    }
    else if (e->message == msgUp && isOn == true) {
        isOn = false;
        if (upAction) upAction();
    }
}

template <EventUseCase T>
static void HandleKeyboardPress(PMSG e, BYTE vkCode, const Action& action) {
    static bool isOn = false;
    if (e->wParam == vkCode && e->message == WM_KEYDOWN && isOn == false) {
        isOn = true;
        if (action) action();
    }
    else if (e->wParam == vkCode && e->message == WM_KEYUP && isOn == true) {
        isOn = false;
    }
}

namespace core::messagequeue {
    HCURSOR WINAPI _SetCursor(HCURSOR hCursor);
    decltype(&_SetCursor) OriSetCursor;
    int WINAPI _ShowCursor(BOOL bShow);
    decltype(&_ShowCursor) OriShowCursor;

    bool isCursorShow = true;
    auto hCursor = LoadCursorA(nil, IDC_ARROW);

    HCURSOR WINAPI _SetCursor(HCURSOR cursor) {
        if (g_showImGui)
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
        // Set cursor visibility to -1, reset cursor to a normal arrow,
        // to ensure that there is a visible mouse cursor on the game's config dialog
        while (OriShowCursor(TRUE) < 0);
        while (OriShowCursor(FALSE) >= 0);
        cursorVisibility = -1;
        ShowMousePointer();
    }

    static InputRuleItemVk2SideEffect InputRuleVk2SideEffect[]{
        { &gs_toggleImGuiButton, 0, [](NOUSE bool _, NOUSE bool __) {
            g_showImGui = !g_showImGui;
            if (g_showImGui) {
                g_inputEnabled = false;
                ShowMousePointer();
            }
            else
                HideMousePointer();
        } },
        { &gs_toggleOsCursorButton, 0, [](NOUSE bool _, NOUSE bool __) {
            isCursorShow ? HideMousePointer() : ShowMousePointer();
        } },
        { &gs_toggleMouseControl, 0, [](NOUSE bool _, bool wantCaptureMouse) {
            g_inputEnabled = wantCaptureMouse ? false : !g_inputEnabled;
            if (!wantCaptureMouse)
                ImGui::SetWindowFocus();
        } },
        { nil, VK_LBUTTON, [](NOUSE bool _, bool wantCaptureMouse) {
            if (wantCaptureMouse)
                g_inputEnabled = false;

        } },
        { nil, VK_MBUTTON, [](NOUSE bool _, bool wantCaptureMouse) {
            if (wantCaptureMouse)
                g_inputEnabled = false;
            else
                ImGui::SetWindowFocus();
        } },
        { nil, VK_RBUTTON, [](bool isUp, bool wantCaptureMouse) {
            if (!isUp) return;
            if (!wantCaptureMouse)
                ImGui::SetWindowFocus();
        } },
    };

    static InputRuleItemMouseBtn2Function InputRuleMouseBtn2ClickState[]{
        { VK_LBUTTON, &g_leftClicked },
        { VK_MBUTTON, &g_middleClicked },
        { VK_RBUTTON, &g_rightClicked },
        { VK_XBUTTON1, &g_forwardClicked },
        { VK_XBUTTON2, &g_backwardClicked },
    };

    static LRESULT CALLBACK GetMsgProcW(int code, WPARAM wParam, LPARAM lParam) {
        using enum EventUseCase;
        auto e = PMSG(lParam);
        if (code == HC_ACTION && g_hookApplied && g_hFocusWindow && e->hwnd == g_hFocusWindow) {



            HandleKeyboardPress<ImGuiBtn>(e, gs_toggleImGuiButton, []() {
                g_showImGui = !g_showImGui;
                if (g_showImGui) {
                    g_inputEnabled = false;
                    ShowMousePointer();
                }
                else
                    HideMousePointer();
                });

            if (g_showImGui) {
                ImGui_ImplWin32_WndProcHandler(e->hwnd, e->message, e->wParam, e->lParam);
            }
            else {
                HandleKeyboardPress<OsCursorBtn>(e, gs_toggleOsCursorButton,
                    []() { isCursorShow ? HideMousePointer() : ShowMousePointer(); });
            }

            HandleMousePress<MouseLeftClick>(e, WM_LBUTTONDOWN, WM_LBUTTONUP, []() {
                auto wantCaptureMouse = g_showImGui && ImGui::GetIO().WantCaptureMouse;
                g_leftClicked = wantCaptureMouse ? false : true;
                if (wantCaptureMouse)
                    g_inputEnabled = false;
                }, []() { g_leftClicked = false; });

            HandleMousePress<MouseMiddleClick>(e, WM_MBUTTONDOWN, WM_MBUTTONUP, []() {
                auto wantCaptureMouse = g_showImGui && ImGui::GetIO().WantCaptureMouse;
                g_middleClicked = wantCaptureMouse ? false : true;
                if (wantCaptureMouse)
                    g_inputEnabled = false;
                else
                    ImGui::SetWindowFocus();
                }, []() { g_middleClicked = false; });

            HandleMousePress<MouseRightClick>(e, WM_RBUTTONDOWN, WM_RBUTTONUP, nil, []() {
                auto wantCaptureMouse = g_showImGui && ImGui::GetIO().WantCaptureMouse;
                g_inputEnabled = wantCaptureMouse ? false : !g_inputEnabled;
                if (!wantCaptureMouse)
                    ImGui::SetWindowFocus();
                });
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