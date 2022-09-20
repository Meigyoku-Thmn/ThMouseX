module;

#include "framework.h"
#include "macro.h"
#include <vector>
#include <Psapi.h>
#include <tlhelp32.h>
#include <string_view>
#include <string>
#include <regex>
#include <tuple>

export module common.helper;

import common.var;
import common.datatype;

using namespace std;

export DLLEXPORT void ReportLastError(const char* title) {
    auto dwErr = GetLastError();
    // lookup error code and display it
    LPVOID lpMsgBuf{};
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwErr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );
    MessageBoxA(NULL, (LPCTSTR)lpMsgBuf, title, MB_OK | MB_ICONERROR);
    // Free the buffer.
    LocalFree(lpMsgBuf);
}

wregex whitespace(LR"([^\s])", wregex::ECMAScript | wregex::optimize);

export DLLEXPORT wstring_view LTrim(wstring_view str) {
    match_results<wstring_view::const_iterator> match;
    auto pos = regex_search(str.cbegin(), str.cend(), match, whitespace) ? match.position(0) : wstring::npos;
    str.remove_prefix(min(pos, str.length()));
    return str;
}

export DLLEXPORT wstring_view RTrim(wstring_view str) {
    match_results<reverse_iterator<wstring_view::const_iterator>> match;
    auto pos = regex_search(str.crbegin(), str.crend(), match, whitespace) ? match.position(0) : wstring::npos;
    str.remove_suffix(min(pos, str.length()));
    return str;
}

export DLLEXPORT wstring_view Trim(wstring_view str) {
    return RTrim(LTrim(str));
}

constexpr UCHAR Delta = 16;
constexpr UCHAR WhiteIntensityLimit = 128;
constexpr UCHAR BlackIntensityLimit = 16;

export DLLEXPORT void CalculateNextModulate(UCHAR& modulate, ModulateStage& modulateStage) {
    switch (modulateStage) {
        case WhiteInc: WhiteInc:
    if (modulate == WhiteIntensityLimit) {
        modulateStage = WhiteDec;
        goto WhiteDec;
    } else {
        modulate += Delta;
    }
    break;
        case WhiteDec: WhiteDec:
    if (modulate == 0) {
        modulateStage = BlackInc;
        modulate = BlackIntensityLimit;
    } else {
        modulate -= Delta;
    }
    break;
        case BlackInc: BlackInc:
    if (modulate == 0) {
        modulateStage = BlackDec;
        goto BlackDec;
    } else {
        modulate -= Delta;
    }
    break;
        case BlackDec: BlackDec:
    if (modulate == BlackIntensityLimit) {
        modulateStage = WhiteInc;
        modulate = 0;
    } else {
        modulate += Delta;
    }
    break;
    }
}

export DLLEXPORT POINT GetPointerPosition() {
    POINT pointerPosition;
    GetCursorPos(&pointerPosition);
    ScreenToClient(g_hFocusWindow, &pointerPosition);
    return pointerPosition;
}

export DLLEXPORT void RemoveWindowBorder(UINT width, UINT height) {
    auto style = GetWindowLongPtrW(g_hFocusWindow, GWL_STYLE);
    if (style == 0)
        return;
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    auto lExStyle = GetWindowLongPtrW(g_hFocusWindow, GWL_EXSTYLE);
    if (lExStyle == 0)
        return;
    lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    if (SetWindowLongPtrW(g_hFocusWindow, GWL_STYLE, style) == 0)
        return;
    if (SetWindowLongPtrW(g_hFocusWindow, GWL_EXSTYLE, lExStyle) == 0)
        return;
    SetWindowPos(g_hFocusWindow, NULL, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
}
