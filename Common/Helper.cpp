#include "framework.h"
#include "macro.h"
#include <string>
#include <tuple>

#include "Helper.h"
#include "Helper.Memory.h"
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
        FormatMessageA(flags, NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), PSTR(&errorMessage), 0, NULL);
        MessageBoxA(NULL, errorMessage, title, MB_OK | MB_ICONERROR);
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
        const char* message = nullptr;
        auto result = strtof(input.c_str(), &endPtr);
        if (errno == ERANGE)
            message = "out of range (type float)";
        else if (endPtr - input.c_str() != input.size() || input.size() == 0)
            message = "invalid format";
        return tuple(result, message);
    }

    tuple<long, const char*> ConvertToLong(const string& input, int base) {
        char* endPtr;
        const char* message = nullptr;
        auto result = strtol(input.c_str(), &endPtr, base);
        if (errno == ERANGE)
            message = "out of range (type long)";
        else if (endPtr - input.c_str() != input.size() || input.size() == 0)
            message = "invalid format";
        return tuple(result, message);
    }

    tuple<unsigned long, const char*> ConvertToULong(const string& input, int base) {
        char* endPtr;
        const char* message = nullptr;
        auto result = strtoul(input.c_str(), &endPtr, base);
        if (errno == ERANGE)
            message = "out of range (type unsigned long)";
        else if (endPtr - input.c_str() != input.size() || input.size() == 0)
            message = "invalid format";
        return tuple(result, message);
    }

    void CalculateNextTone(UCHAR& tone, ModulateStage& toneStage) {
        constexpr UCHAR Delta = 16;
        constexpr UCHAR WhiteIntensityLimit = 128;
        constexpr UCHAR BlackIntensityLimit = 16;
        switch (toneStage) {
            case WhiteInc:
                if (tone == WhiteIntensityLimit) {
                    toneStage = WhiteDec;
                    goto WhiteDec;
                } else {
                    tone += Delta;
                }
                break;
            case WhiteDec: WhiteDec:
                if (tone == 0) {
                    toneStage = BlackInc;
                    tone = BlackIntensityLimit;
                } else {
                    tone -= Delta;
                }
                break;
            case BlackInc:
                if (tone == 0) {
                    toneStage = BlackDec;
                    goto BlackDec;
                } else {
                    tone -= Delta;
                }
                break;
            case BlackDec: BlackDec:
                if (tone == BlackIntensityLimit) {
                    toneStage = WhiteInc;
                    tone = 0;
                } else {
                    tone += Delta;
                }
                break;
        }
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
        SetWindowPos(g_hFocusWindow, NULL, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
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
            SetWindowPos(g_hFocusWindow, NULL, 0, 0, d3dWidth, d3dHeight, updateFlags);
        } else if (d3dWidth > clientWidth || d3dHeight > clientHeight) {
            // fix for Touhou 18
            RECTSIZE size{0, 0, LONG(d3dWidth), LONG(d3dHeight)};
            auto style = GetWindowLongPtrW(g_hFocusWindow, GWL_STYLE);
            auto hasMenu = GetMenu(g_hFocusWindow) != NULL ? TRUE : FALSE;
            auto exStyle = GetWindowLongPtrW(g_hFocusWindow, GWL_EXSTYLE);
            AdjustWindowRectEx(&size, style, hasMenu, exStyle);
            auto updateFlags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOREPOSITION;
            SetWindowPos(g_hFocusWindow, NULL, 0, 0, size.width(), size.height(), updateFlags);
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
        if (g_currentConfig.ScriptType == ScriptType::LuaJIT)
            return luajit::GetPositionAddress();
        else if (g_currentConfig.ScriptType == ScriptType::NeoLua)
            return neolua::GetPositionAddress();
        else if (g_currentConfig.ScriptType == ScriptType::Lua)
            return lua::GetPositionAddress();
        else
            return memory::ResolveAddress(g_currentConfig.Address.Level, g_currentConfig.Address.Length);        
    }
}
