#include "../Common/macro.h"
#include "../Common/DataTypes.h"
#include <functional>
#include <vector>
#include <Windows.h>

#include "../Common/Log.h"
#include "Intercom.h"

using namespace std;

namespace note = common::log;

#define TAG "[Intercom] "

static const auto ServerWindowName = L"{BC9D7991-41A0-47F3-BC4D-6CFF2BA205FD}";
static const auto ClientClassName = L"{E4CB816C-3BF1-41F5-BAC5-4D4B847846A7}";

constexpr auto GET_CONFIG_EVENT = 1;
constexpr auto GET_CONFIG_EVENT_RETURN = 2;
constexpr auto GET_MEM_BLOCK_EVENT_RETURN = 3;

constexpr auto GET_MEM_BLOCK_MSG = WM_APP + 1;

constexpr auto Timeout = 1000 * 60 * 10;
constexpr auto TimeoutFlag = SMTO_NOTIMEOUTIFNOTHUNG | SMTO_ERRORONEXIT;

static HWND CreateResultWindow();
static void DestroyResultWindow(HWND hwnd);
static bool RequestGameConfig(LPCWSTR procName, HWND server, HWND client, CommonConfig& commonConfig, GameConfig& gameConfig);
static bool RequestMemBlock(PVOID dst, HWND serverHwnd, HWND clientHwnd);

namespace core::intercom {
    bool QueryGameConfig(LPCWSTR processName, CommonConfig& commonConfig, GameConfig& gameConfig) {
        using enum ScriptType;
        SetLastError(ERROR_SUCCESS);
        auto serverHwnd = FindWindowExW(HWND_MESSAGE, nil, nil, ServerWindowName);
        if (serverHwnd == nil) {
            if (GetLastError() == ERROR_SUCCESS)
                note::ToFile(TAG "Cannot find the server window: No window matches the name.");
            else
                note::LastErrorToFile(TAG "Cannot find the server window");
            return false;
        }

        auto clientHwnd = CreateResultWindow();
        if (clientHwnd == nil)
            return false;
        defer({ DestroyResultWindow(clientHwnd); });

        if (!RequestGameConfig(processName, serverHwnd, clientHwnd, commonConfig, gameConfig))
            return false;
        if (!RequestMemBlock(&commonConfig.TextureFilePath, serverHwnd, clientHwnd))
            return false;
        if (!RequestMemBlock(&commonConfig.ImGuiFontPath, serverHwnd, clientHwnd))
            return false;
        if (!RequestMemBlock(&gameConfig.ProcessName, serverHwnd, clientHwnd))
            return false;
        if (gameConfig.ScriptType == None && !RequestMemBlock(&gameConfig.Offsets, serverHwnd, clientHwnd))
            return false;

        return true;
    }
}

static function<bool(const COPYDATASTRUCT*)> copyDataCallback;

static HWND CreateResultWindow() {
    WNDCLASSW windowClass{
        .lpfnWndProc = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
            if (uMsg == WM_COPYDATA && copyDataCallback != nil && copyDataCallback(bcast<const COPYDATASTRUCT*>(lParam)))
                return LRESULT(TRUE);
            return DefWindowProcW(hWnd, uMsg, wParam, lParam);
        },
        .lpszClassName = ClientClassName,
    };
    if (!RegisterClassW(&windowClass)) {
        note::LastErrorToFile(TAG "Cannot register class of the result client window");
        return nil;
    }
    auto messageWindow = CreateWindowW(ClientClassName, ClientClassName, 0, 0, 0, 0, 0, HWND_MESSAGE, nil, nil, nil);
    if (!messageWindow) {
        note::LastErrorToFile(TAG "Cannot create the result client window");
        UnregisterClassW(ClientClassName, nil);
        return nil;
    }
    return messageWindow;
}

static void DestroyResultWindow(HWND hwnd) {
    DestroyWindow(hwnd);
    UnregisterClassW(ClientClassName, nil);
}

static bool RequestGameConfig(LPCWSTR procName, HWND server, HWND client, CommonConfig& commonConfig, GameConfig& gameConfig) {
    copyDataCallback = [&](const COPYDATASTRUCT* received) {
        if (received->dwData != GET_CONFIG_EVENT_RETURN)
            return false;
        commonConfig = *bcast<CommonConfig*>(received->lpData);
        gameConfig = *bcast<GameConfig*>(bcast<uintptr_t>(received->lpData) + sizeof(CommonConfig));
        return true;
    };
    COPYDATASTRUCT data{
        .dwData = GET_CONFIG_EVENT,
        .cbData = scast<DWORD>((wcslen(procName) + 1) * sizeof(procName[0])),
        .lpData = bcast<PVOID>(procName),
    };
    SetLastError(ERROR_SUCCESS);
    DWORD_PTR serverResult;
    auto result = SendMessageTimeoutW(server, WM_COPYDATA, bcast<WPARAM>(client),
        bcast<LPARAM>(&data), TimeoutFlag, Timeout, &serverResult);
    copyDataCallback = nil;
    if (result == 0) {
        note::LastErrorToFile(TAG "Failed to get game config from the server window");
        return false;
    }
    if (serverResult == 0)
        return false;
    return true;
}

static vector<CMemHandler> memBlockPtrs;
struct StartupCode {
    StartupCode() {
        memBlockPtrs.reserve(3);
    }
};
static StartupCode _;

static bool RequestMemBlock(PVOID dst, HWND serverHwnd, HWND clientHwnd) {
    auto& dest = *bcast<PVOID*>(dst);
    copyDataCallback = [&](const COPYDATASTRUCT* received) {
        if (received->dwData != GET_MEM_BLOCK_EVENT_RETURN)
            return false;
        dest = malloc(received->cbData);
        if (dest == nil)
            return false;
        memBlockPtrs.emplace_back(dest);
        memcpy(dest, received->lpData, received->cbData);
        return true;
    };
    SetLastError(ERROR_SUCCESS);
    DWORD_PTR serverResult;
    auto result = SendMessageTimeoutW(serverHwnd, GET_MEM_BLOCK_MSG, bcast<WPARAM>(clientHwnd),
        bcast<LPARAM>(dest), TimeoutFlag, Timeout, &serverResult);
    copyDataCallback = nil;
    if (result == 0) {
        note::LastErrorToFile(TAG "Failed to get memory block from the server window");
        return false;
    }
    if (serverResult == 0)
        return false;
    return true;
}