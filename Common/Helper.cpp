#include <Windows.h>
#include "macro.h"
#include <string>
#include <tuple>
#include <span>
#include <format>
#include <atomic>

#include "Helper.h"
#include "Helper.Memory.h"
#include "Helper.Encoding.h"
#include "Log.h"
#include "Lua.h"
#include "LuaJIT.h"
#include "NeoLua.h"
#include "Variables.h"

using namespace std;

namespace common::helper {
    void ReportLastError(const char* title) {
        auto flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        auto dwErr = GetLastError();
        PSTR errorMessage{};
        FormatMessageA(flags, nil, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), PSTR(&errorMessage), 0, nil);
        MessageBoxA(nil, errorMessage, title, MB_OK | MB_ICONERROR);
        LocalFree(errorMessage);
    }

    string& Replace(string& input, const char* keyword, const char* replacement) {
        size_t keywordPos = 0;
        auto keywordLen = strlen(keyword);
        auto replacementLen = strlen(replacement);
        while ((keywordPos = input.find(keyword, keywordPos)) != string::npos) {
            input.replace(keywordPos, keywordLen, replacement);
            keywordPos += replacementLen;
        }
        return input;
    }

    tuple<float, const char*> ConvertToFloat(const string& input) {
        char* endPtr;
        const char* message = nil;
        auto result = strtof(input.c_str(), &endPtr);
        if (errno == ERANGE)
            message = "out of range (type float)";
        else if (endPtr - input.c_str() != input.size() || input.size() == 0)
            message = "invalid format";
        return tuple(result, message);
    }

    tuple<long, const char*> ConvertToLong(const string& input, int base) {
        char* endPtr;
        const char* message = nil;
        auto result = strtol(input.c_str(), &endPtr, base);
        if (errno == ERANGE)
            message = "out of range (type long)";
        else if (endPtr - input.c_str() != input.size() || input.size() == 0)
            message = "invalid format";
        return tuple(result, message);
    }

    tuple<unsigned long, const char*> ConvertToULong(const string& input, int base) {
        char* endPtr;
        const char* message = nil;
        auto result = strtoul(input.c_str(), &endPtr, base);
        if (errno == ERANGE)
            message = "out of range (type unsigned long)";
        else if (endPtr - input.c_str() != input.size() || input.size() == 0)
            message = "invalid format";
        return tuple(result, message);
    }

    void CalculateNextTone(UCHAR& tone, ModulateStage& toneStage) {
        using enum ModulateStage;
        constexpr UCHAR Delta = 16;
        constexpr UCHAR WhiteIntensityLimit = 128;
        constexpr UCHAR BlackIntensityLimit = 16;
        bool doContinue = false;
        do {
            doContinue = false;
            switch (toneStage) {
                case WhiteInc:
                    if (tone == WhiteIntensityLimit) {
                        toneStage = WhiteDec;
                        doContinue = true;
                    }
                    else {
                        tone += Delta;
                    }
                    break;
                case WhiteDec:
                    if (tone == 0) {
                        toneStage = BlackInc;
                        tone = BlackIntensityLimit;
                    }
                    else {
                        tone -= Delta;
                    }
                    break;
                case BlackInc:
                    if (tone == 0) {
                        toneStage = BlackDec;
                        doContinue = true;
                    }
                    else {
                        tone -= Delta;
                    }
                    break;
                case BlackDec:
                    if (tone == BlackIntensityLimit) {
                        toneStage = WhiteInc;
                        tone = 0;
                    }
                    else {
                        tone += Delta;
                    }
                    break;
            }
        } while (doContinue);
    }

    POINT GetPointerPosition() {
        POINT pointerPosition;
        GetCursorPos(&pointerPosition);
        ScreenToClient(g_hFocusWindow, &pointerPosition);
        return pointerPosition;
    }

    void RemoveWindowBorder(UINT width, UINT height) {
        auto style = GetWindowLongPtrW(g_hFocusWindow, GWL_STYLE);
        style &= ~(WS_CAPTION | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        auto lExStyle = GetWindowLongPtrW(g_hFocusWindow, GWL_EXSTYLE);
        lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
        SetWindowLongPtrW(g_hFocusWindow, GWL_STYLE, style);
        SetWindowLongPtrW(g_hFocusWindow, GWL_EXSTYLE, lExStyle);
        SetWindowPos(g_hFocusWindow, nil, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
    }

    void FixWindowCoordinate(bool isExclusiveMode, UINT d3dWidth, UINT d3dHeight, UINT clientWidth, UINT clientHeight) {
        if (isExclusiveMode) {
            auto style = GetWindowLongPtrW(g_hFocusWindow, GWL_STYLE);
            style &= ~(WS_CAPTION | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
            auto exStyle = GetWindowLongPtrW(g_hFocusWindow, GWL_EXSTYLE);
            exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
            SetWindowLongPtrW(g_hFocusWindow, GWL_STYLE, style);
            SetWindowLongPtrW(g_hFocusWindow, GWL_EXSTYLE, exStyle);
            auto updateFlags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER;
            SetWindowPos(g_hFocusWindow, nil, 0, 0, d3dWidth, d3dHeight, updateFlags);
        }
        else if (d3dWidth > clientWidth || d3dHeight > clientHeight) {
            // fix for Touhou 18
            RECTSIZE size{ 0, 0, LONG(d3dWidth), LONG(d3dHeight) };
            auto style = GetWindowLongPtrW(g_hFocusWindow, GWL_STYLE);
            auto hasMenu = GetMenu(g_hFocusWindow) != nil ? TRUE : FALSE;
            auto exStyle = GetWindowLongPtrW(g_hFocusWindow, GWL_EXSTYLE);
            AdjustWindowRectEx(&size, style, hasMenu, exStyle);
            auto updateFlags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOREPOSITION;
            SetWindowPos(g_hFocusWindow, nil, 0, 0, size.width(), size.height(), updateFlags);
        }
    }

    // use for directx8
    bool TestFullscreenHeuristically() {
        MONITORINFO monitorInfo{
            .cbSize = sizeof(MONITORINFO),
        };
        GetMonitorInfoW(MonitorFromWindow(g_hFocusWindow, MONITOR_DEFAULTTOPRIMARY), &monitorInfo);

        RECT hwndRect;
        GetWindowRect(g_hFocusWindow, &hwndRect);

        return hwndRect.left == monitorInfo.rcMonitor.left
            && hwndRect.right == monitorInfo.rcMonitor.right
            && hwndRect.top == monitorInfo.rcMonitor.top
            && hwndRect.bottom == monitorInfo.rcMonitor.bottom;
    }

    DWORD CalculateAddress() {
        if (g_gameConfig.ScriptType == ScriptType_LuaJIT)
            return luajit::GetPositionAddress();
        else if (g_gameConfig.ScriptType == ScriptType_NeoLua)
            return neolua::GetPositionAddress();
        else if (g_gameConfig.ScriptType == ScriptType_Lua)
            return lua::GetPositionAddress();
        else {
            auto& addressChain = g_gameConfig.AddressChain;
            return memory::ResolveAddress(span{ &addressChain[addressChain.GetLowerBound()], addressChain.GetCount() });
        }
    }

    bool IsCurrentProcessThMouseX() {
        auto envVal = getenv(APP_NAME);
        if (envVal == nil)
            return false;
        return strcmp(envVal, APP_NAME) == 0;
    }

    tuple<DWORD, string> CallProcess(const wstring& processPath, const wstring& cmdLine) {
        STARTUPINFOW info = { sizeof(info) };
        PROCESS_INFORMATION processInfo;
        wstring _cmdLine = L"\"" + processPath + L"\" " + cmdLine;
        auto rs = CreateProcessW(processPath.c_str(), _cmdLine.data(), nil, nil, FALSE, 0, nil, nil, &info, &processInfo);
        if (!rs)
            return tuple(-1, format("Failed to create process '{}'", encoding::ConvertToUtf8(processPath)));
        WaitForSingleObject(processInfo.hProcess, INFINITE);
        DWORD exitCode;
        rs = GetExitCodeProcess(processInfo.hProcess, &exitCode);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        if (!rs)
            return tuple(-1, format("Failed to get the exit code of process '{}'", encoding::ConvertToUtf8(processPath)));
        return tuple(exitCode, string());
    }

    // from Autoit source code: https://github.com/ellysh/au3src/blob/35517393091e7d97052d20ccdee8d9d6db36276f/src/sendkeys.cpp#L790
    bool ShouldBeVkExtended(BYTE key) {
        return
            key == VK_INSERT || key == VK_DELETE || key == VK_END || key == VK_DOWN ||
            key == VK_NEXT || key == VK_LEFT || key == VK_RIGHT || key == VK_HOME || key == VK_UP ||
            key == VK_PRIOR || key == VK_DIVIDE || key == VK_APPS || key == VK_LWIN || key == VK_RWIN ||
            key == VK_RMENU || key == VK_RCONTROL || key == VK_SLEEP || key == VK_BROWSER_BACK ||
            key == VK_BROWSER_FORWARD || key == VK_BROWSER_REFRESH || key == VK_BROWSER_STOP ||
            key == VK_BROWSER_SEARCH || key == VK_BROWSER_FAVORITES || key == VK_BROWSER_HOME ||
            key == VK_VOLUME_MUTE || key == VK_VOLUME_DOWN || key == VK_VOLUME_UP || key == VK_MEDIA_NEXT_TRACK ||
            key == VK_MEDIA_PREV_TRACK || key == VK_MEDIA_STOP || key == VK_MEDIA_PLAY_PAUSE ||
            key == VK_LAUNCH_MAIL || key == VK_LAUNCH_MEDIA_SELECT || key == VK_LAUNCH_APP1 || key == VK_LAUNCH_APP2 ||
            key == VK_CANCEL;
    }

    BYTE MapVk2Dik(BYTE vkCode, PBYTE mappingTable, BYTE defaultDikCode) {
        if (vkCode == 0)
            return defaultDikCode;
        auto scancode = (BYTE)MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC_EX);
        if (vkCode == VK_PAUSE)
            scancode = 0x45;
        else if (vkCode == VK_NUMLOCK && scancode == 0x45 || ShouldBeVkExtended(vkCode))
            scancode |= 0x80;
        if (scancode == 0)
            return defaultDikCode;
        return mappingTable[scancode];
    }

    BYTE NormalizeLeftRightVkCode(BYTE vkCode) {
        if (vkCode == VK_LCONTROL || vkCode == VK_RCONTROL)
            return VK_CONTROL;
        if (vkCode == VK_LSHIFT || vkCode == VK_RSHIFT)
            return VK_SHIFT;
        if (vkCode == VK_LMENU || vkCode == VK_RMENU)
            return VK_MENU;
        return vkCode;
    }

    VkCodeMessage ConvertVkCodeToMessage(BYTE vkCode) {
        if (vkCode == VK_LBUTTON)
            return VkCodeMessage(
                WM_LBUTTONUP, nil, nil,
                WM_LBUTTONDOWN, nil, nil
            );
        if (vkCode == VK_MBUTTON)
            return VkCodeMessage(
                WM_MBUTTONUP, nil, nil,
                WM_MBUTTONDOWN, nil, nil
            );
        if (vkCode == VK_RBUTTON)
            return VkCodeMessage(
                WM_RBUTTONUP, nil, nil,
                WM_RBUTTONDOWN, nil, nil
            );
        if (vkCode == VK_XBUTTON1)
            return VkCodeMessage(
                WM_XBUTTONUP, [](auto _wParam, UNUSED auto _) { return (_wParam & (XBUTTON1 << 16)) != 0; }, nil,
                WM_XBUTTONDOWN, [](auto _wParam, UNUSED auto _) { return (_wParam & (XBUTTON1 << 16)) != 0; }, nil
            );
        if (vkCode == VK_XBUTTON2)
            return VkCodeMessage(
                WM_XBUTTONUP, [](auto _wParam, UNUSED auto _) { return (_wParam & (XBUTTON2 << 16)) != 0; }, nil,
                WM_XBUTTONDOWN, [](auto _wParam, UNUSED auto _) { return (_wParam & (XBUTTON2 << 16)) != 0; }, nil
            );
        if (vkCode == SCROLL_UP_EVENT)
            return VkCodeMessage(
                WM_NULL, nil, nil,
                WM_MOUSEWHEEL, [](auto _wParam, UNUSED auto _) { return INT16(_wParam >> 16) > 0; }, nil
            );
        if (vkCode == SCROLL_DOWN_EVENT)
            return VkCodeMessage(
                WM_NULL, nil, nil,
                WM_MOUSEWHEEL, [](auto _wParam, UNUSED auto _) { return INT16(_wParam >> 16) < 0; }, nil
            );
        if (vkCode == SCROLL_LEFT_EVENT)
            return VkCodeMessage(
                WM_NULL, nil, nil,
                WM_MOUSEHWHEEL, [](auto _wParam, UNUSED auto _) { return INT16(_wParam >> 16) < 0; }, nil
            );
        if (vkCode == SCROLL_RIGHT_EVENT)
            return VkCodeMessage(
                WM_NULL, nil, nil,
                WM_MOUSEHWHEEL, [](auto _wParam, UNUSED auto _) { return INT16(_wParam >> 16) > 0; }, nil
            );
        return VkCodeMessage(
            WM_KEYUP, [](auto _wParam, auto _vkCode) { return _wParam == _vkCode; }, nil,
            WM_KEYDOWN, [](auto _wParam, auto _vkCode) { return _wParam == _vkCode; }, nil
        );
    }

    // If facility of hresult is win32 then return the lower 16bit, else return unchanged
    HRESULT Win32FromHResult(HRESULT hr) {
        if (SUCCEEDED(hr))
            return ERROR_SUCCESS;
        if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
            return HRESULT_CODE(hr);
        return hr;
    }

    void SafeFreeLib(HMODULE& hLibModule) {
        if (hLibModule) {
            FreeLibrary(hLibModule);
            hLibModule = nil;
        }
    }

    wstring ExpandEnvStr(const wchar_t* str) {
        auto chrCount = ExpandEnvironmentStringsW(str, nil, 0);
        if (chrCount == 0)
            return wstring();
        wstring output(chrCount - 1, '\0');
        ExpandEnvironmentStringsW(str, output.data(), chrCount);
        return output;
    }

    wstring ExpandEnvStr(const wstring& str) {
        return ExpandEnvStr(str.c_str());
    }

    void ComMethodTimeout(const function<void()>& comAction, DWORD timeout) {
        atomic<DWORD> mainThreadId = GetCurrentThreadId();
        Handle waitHandle{ CreateEventW(nullptr, TRUE, FALSE, nullptr) };
        if (waitHandle == nil) {
            log::LastErrorToFile("CreateEventW failed");
            return;
        }
        auto timeoutCallback = [&] {
            auto _mainThreadId = mainThreadId.exchange(0);
            if (_mainThreadId == 0)
                return;
            auto hr = CoCancelCall(_mainThreadId, 0);
            if (FAILED(hr))
                log::HResultToFile("CoCancelCall failed", hr);
            SetEvent(waitHandle.get());
        };
        auto timeoutCallbackThunk = [](auto callback, UNUSED auto _) {
            (*(decltype(timeoutCallback)*)callback)();
        };
        auto timerHandle = CreateTimerQueueTimer(nullptr, timeoutCallbackThunk, &timeoutCallback, timeout, 0, WT_EXECUTEONLYONCE);
        if (!timerHandle) {
            log::LastErrorToFile("CreateTimerQueueTimer failed");
            return;
        }
        auto hr = CoEnableCallCancellation(nullptr);
        if (FAILED(hr)) {
            log::HResultToFile("CoEnableCallCancellation failed", hr);
            return;
        }
        comAction();
        auto _mainThreadId = mainThreadId.exchange(0);
        if (_mainThreadId == 0)
            WaitForSingleObject(waitHandle.get(), INFINITE);
        hr = CoDisableCallCancellation(nullptr);
        if (FAILED(hr))
            log::HResultToFile("CoDisableCallCancellation failed", hr);
    }

    TimerQueueTimerHandle CreateTimerQueueTimer(HANDLE TimerQueue, WAITORTIMERCALLBACK Callback, PVOID Parameter, DWORD DueTime, DWORD Period, ULONG Flags) {
        HANDLE timerHandle{};
        auto rs = ::CreateTimerQueueTimer(&timerHandle, TimerQueue, Callback, Parameter, DueTime, Period, Flags);
        if (!rs)
            timerHandle = nullptr;
        return TimerQueueTimerHandle{ timerHandle };
    }

    ActCtxCookie ActivateActCtx(HANDLE hActCtx) {
        ULONG_PTR cookie{};
        auto rs = ::ActivateActCtx(hActCtx, &cookie);
        if (!rs)
            cookie = 0;
        return ActCtxCookie{ cookie };
    }
}
