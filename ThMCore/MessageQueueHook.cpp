#include "framework.h"
#include <vector>
#include <imgui_impl_win32.h>

#include "../Common/macro.h"
#include "../Common/MinHook.h"
#include "../Common/Variables.h"
#include "../Common/Helper.h"
#include "../Common/NeoLua.h"
#include "../Common/CallbackStore.h"
#include "Initialization.h"
#include "MessageQueueHook.h"

namespace minhook = common::minhook;
namespace neolua = common::neolua;
namespace helper = common::helper;
namespace callbackstore = common::callbackstore;

using namespace std;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace core::messagequeuehook {
    UINT CLEAN_MANAGED_DATA = RegisterWindowMessageA("CLEAN_MANAGED_DATA {6BF7C2B8-F245-4781-AA3C-467366CA3551}");
    bool TestRegisteredWindowMessages() {
        if (CLEAN_MANAGED_DATA == 0) {
            MessageBoxA(NULL, "Failed to register CLEAN_MANAGED_DATA message.", "RegisterWindowMessage error", MB_OK | MB_ICONERROR);
            return false;
        }
        return true;
    }

    HCURSOR WINAPI _SetCursor(HCURSOR hCursor);
    decltype(&_SetCursor) OriSetCursor;
    int WINAPI _ShowCursor(BOOL bShow);
    decltype(&_ShowCursor) OriShowCursor;

    bool isCursorShow = true;
    auto hCursor = LoadCursorA(NULL, IDC_ARROW);

    vector<minhook::HookApiConfig> HookConfig{
        { L"USER32.DLL", "SetCursor", & _SetCursor, (PVOID*)&OriSetCursor },
        { L"USER32.DLL", "ShowCursor", &_ShowCursor, (PVOID*)&OriShowCursor },
    };

    HCURSOR WINAPI _SetCursor(HCURSOR hCursor) {
        return NULL;
    }

    int WINAPI _ShowCursor(BOOL bShow) {
        return bShow == TRUE ? 0 : -1;
    }

    bool cursorNormalized;
    int cursorVisibility;
    void ShowCursorEx(bool show) {
        if (show && cursorVisibility < 0)
            OriShowCursor(TRUE);
        else if (!show && cursorVisibility >= 0)
            OriShowCursor(FALSE);
    }

    void HideMousePointer() {
        OriSetCursor(NULL);
        ShowCursorEx(false);
        isCursorShow = false;
    }

    void ShowMousePointer() {
        OriSetCursor(hCursor);
        ShowCursorEx(true);
        isCursorShow = true;
    }

    struct OnInit {
        OnInit() {
            // Hide the mouse cursor when D3D is running, but only after cursor normalization
            callbackstore::RegisterPostRenderCallback(Callback);
        }
        static void Callback() {
            static bool callbackDone = false;
            if (cursorNormalized && !callbackDone) {
                callbackDone = true;
                HideMousePointer();
            }
        }
    } _;

    void NormalizeCursor() {
        // Set cursor visibility to -1, reset cursor to a normal arrow,
        // to ensure that there is a visible mouse cursor on the game's config dialog
        while (OriShowCursor(TRUE) < 0);
        while (OriShowCursor(FALSE) >= 0);
        cursorVisibility = -1;
        ShowMousePointer();
    }

    LRESULT CALLBACK GetMsgProcW(int code, WPARAM wParam, LPARAM lParam) {
        if (code == HC_ACTION && g_hookApplied) {
            auto e = (PMSG)lParam;
            if (g_hFocusWindow != NULL && e->message == WM_KEYDOWN && e->wParam == gs_toggleImGuiButton) {
                g_showImGui = !g_showImGui;
                if (g_showImGui) {
                    g_inputEnabled = false;
                    ShowMousePointer();
                }
                else
                    HideMousePointer();
            }
            if (g_showImGui)
                ImGui_ImplWin32_WndProcHandler(e->hwnd, e->message, e->wParam, e->lParam);
            else {
                static bool isRightMousePressing = false;
                if (e->message == WM_LBUTTONDOWN)
                    g_leftMousePressed = true;
                else if (e->message == WM_MBUTTONDOWN)
                    g_midMousePressed = true;
                else if (e->message == WM_RBUTTONDOWN)
                    isRightMousePressing = true;
                else if (e->message == WM_RBUTTONUP && isRightMousePressing == true) {
                    isRightMousePressing = false;
                    g_inputEnabled = !g_inputEnabled;
                }
                else if (e->message == WM_KEYDOWN) {
                    if (e->wParam == gs_toggleOsCursorButton) {
                        if (isCursorShow)
                            HideMousePointer();
                        else
                            ShowMousePointer();
                    }
                }
            }
        }
        return CallNextHookEx(NULL, code, wParam, lParam);
    }

    LRESULT CALLBACK CallWndRetProcW(int code, WPARAM wParam, LPARAM lParam) {
        if (code == HC_ACTION && g_hookApplied) {
            static auto initialized = false;
            if (!initialized) {
                initialized = true;
                core::Initialize();
            }
            if (!cursorNormalized) {
                cursorNormalized = true;
                NormalizeCursor();
            }
            auto e = (PCWPRETSTRUCT)lParam;
            if (e->message == CLEAN_MANAGED_DATA) {
                neolua::Uninitialize();
            }
            else if (e->message == WM_SETCURSOR) {
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
            else if (e->message == WM_SIZE) {
                if (e->wParam == SIZE_RESTORED) {
                    callbackstore::TriggerClearMeasurementFlagsCallbacks();
                }
            }
        }
        return CallNextHookEx(NULL, code, wParam, lParam);
    }

    bool CheckHookProcHandle(HHOOK handle) {
        if (handle != NULL)
            return true;
        helper::ReportLastError("ThMouseX: SetWindowsHookEx Error");
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
        DWORD _;
        auto broadcastFlags = SMTO_ABORTIFHUNG | SMTO_NOTIMEOUTIFNOTHUNG;
        // notify targets to clean up managed data, but managed DLLs/assemblies unfortunately cannot be unloaded.
        SendMessageTimeoutW(HWND_BROADCAST, CLEAN_MANAGED_DATA, 0, 0, broadcastFlags, 1000, &_);
        // unregister hooks.
        UnhookWindowsHookEx(GetMsgProcHandle);
        UnhookWindowsHookEx(CallWndRetProcHandle);
        // force all top-level windows to process a message, therefore force all processes to unload the DLL.
        SendMessageTimeoutW(HWND_BROADCAST, WM_NULL, 0, 0, broadcastFlags, 1000, &_);
    }
}